#include "system/windows_viewport_manager.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "error/ignis.hpp"
#include "graphics/format.hpp"
#include "graphics/gl_graphics.hpp"
#include "graphics/surface/wgl_swapchain.hpp"
#include "graphics/wglext.h"

#pragma comment(lib, "opengl32.lib")

using namespace oic;
using namespace windows;

#define WGL_FUNC(functionName, FUNCTIONNAME) \
PFN##FUNCTIONNAME##PROC functionName = (PFN##FUNCTIONNAME##PROC)wglGetProcAddress(#functionName); \
if(!functionName) \
	oic::System::log()->fatal(String(#functionName) + " not found");

namespace ignis {

	//Create a swapchain
	Swapchain::Swapchain(Graphics &g, const String &name, const Info &info):
		Surface(g, name, Surface::Info(
			info.vi->size,
			{ GPUFormat::RGBA8 },
			DepthFormat::NONE,
			false
		)), swapchainInfo(info)
	{

		//Fill data buffer
		WNDCLASSA tempWClass{};
		tempWClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		tempWClass.lpfnWndProc = DefWindowProcA;
		tempWClass.hInstance = GetModuleHandle(0);
		tempWClass.lpszClassName = "WGL";

		if(!RegisterClassA(&tempWClass))
			oic::System::log()->fatal(errors::surface::contextError);

		HWND tempWind = CreateWindowExA(
			0,
			tempWClass.lpszClassName,
			"WGL Dummy",
			0,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			0, 0,
			tempWClass.hInstance,
			0
		);

		data = new Swapchain::Data{};
		data->dc = GetDC(tempWind);

		//Create temporary context to query for extensions and versions

		PIXELFORMATDESCRIPTOR pfd;

		ZeroMemory(&pfd, sizeof(pfd));

		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.cColorBits = pfd.cDepthBits = 32;
		pfd.iPixelType = PFD_TYPE_RGBA;

		pfd.dwFlags =
			PFD_DRAW_TO_WINDOW |
			PFD_SUPPORT_OPENGL |
			PFD_GENERIC_ACCELERATED |
			PFD_DOUBLEBUFFER |
			PFD_SWAP_LAYER_BUFFERS;

		//Choose pixel format

		int pixelFormat = ChoosePixelFormat(data->dc, &pfd);

		if (!pixelFormat || !SetPixelFormat(data->dc, pixelFormat, &pfd))
			oic::System::log()->fatal(errors::surface::contextError);

		//Create render context

		data->rc = wglCreateContext(data->dc);

		if(!data->rc || !wglMakeCurrent(data->dc, data->rc))
			oic::System::log()->fatal(errors::surface::contextError);

		//Obtain the OpenGL version and max supported sample count

		GLint samples, major, minor;
		glGetIntegerv(GL_MAX_SAMPLES, &samples);
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		g.getData()->major = u32(major);
		g.getData()->minor = u32(minor);
		g.getData()->maxSamples = u32(samples);

		if (g.getData()->swapchain)
			oic::System::log()->fatal("Each window can only have one swapchain");

		g.getData()->swapchain = this;

		//Get a non core context

		const int pixelAttribs[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0
		};

		//TODO: Depth stencil depends on swapchain::info

		for (auto &elem : glFunctionNames) {

			*elem.second = wglGetProcAddress(elem.first.c_str());

			if(!elem.second)
				oic::System::log()->warn(String("GL Function not found ") + elem.first);
		}

		WGL_FUNC(wglChoosePixelFormatARB, WGLCHOOSEPIXELFORMATARB);
		WGL_FUNC(wglCreateContextAttribsARB, WGLCREATECONTEXTATTRIBSARB);
		WGL_FUNC(wglSwapIntervalEXT, WGLSWAPINTERVALEXT);

		wglMakeCurrent(data->dc, NULL);
		wglDeleteContext(data->rc);
		DestroyWindow(tempWind);
		UnregisterClassA("WGL", GetModuleHandleA(NULL));

		//Create real context

		WWindow *win = (( WViewportManager *) System::viewportManager())->get(info.vi);
		data->dc = GetDC(win->hwnd);

		GLint pixelFormatId{};
		GLuint numFormats{};

		wglChoosePixelFormatARB(data->dc, pixelAttribs, NULL, 1, &pixelFormatId, &numFormats);

		if (!numFormats)
			oic::System::log()->fatal(errors::surface::contextError);

		ZeroMemory(&pfd, sizeof(pfd));

		if (!DescribePixelFormat(data->dc, pixelFormatId, sizeof(pfd), &pfd))
			oic::System::log()->fatal(errors::surface::contextError);

		if (!SetPixelFormat(data->dc, pixelFormatId, &pfd)) {
			HRESULT r = GetLastError();
			r;
			oic::System::log()->fatal(errors::surface::contextError);
		}

		int  contextAttribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, major,
			WGL_CONTEXT_MINOR_VERSION_ARB, minor,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		wglSwapIntervalEXT(swapchainInfo.useVSync);

		data->rc = wglCreateContextAttribsARB(data->dc, 0, contextAttribs);

		if (!data->rc || !wglMakeCurrent(data->dc, data->rc))
			oic::System::log()->fatal(errors::surface::contextError);

		//Call the size update

		onResize(getInfo().size);
	}

	Swapchain::~Swapchain() {

		wglMakeCurrent(data->dc, NULL);
		wglDeleteContext(data->rc);

		WWindow *win = ((WViewportManager*)System::viewportManager())->get(swapchainInfo.vi);
		ReleaseDC(win->hwnd, data->dc);

		destroy(data);
	}

	void Swapchain::present() {
		SwapBuffers(data->dc);
	}
}