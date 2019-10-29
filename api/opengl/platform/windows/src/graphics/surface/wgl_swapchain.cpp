#include "system/windows_viewport_manager.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "error/ignis.hpp"
#include "graphics/format.hpp"
#include "graphics/gl_graphics.hpp"
#include "graphics/surface/wgl_swapchain.hpp"

#pragma comment(lib, "opengl32.lib")

using namespace oic;
using namespace windows;

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
		data = new Swapchain::Data{};

		if (g.getData()->swapchain)
			oic::System::log()->fatal("Each window can only have one swapchain");

		g.getData()->swapchain = this;

		//Create context

		WWindow *win = ((WViewportManager*) System::viewportManager())->get(info.vi);
		data->dc = GetDC(win->hwnd);

		GLint pixelFormatId{};
		GLuint numFormats{};

		//Get a non core context

		const int pixelAttribs[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, int(FormatHelper::getDepthBits(this->info.depthFormat)),
			WGL_STENCIL_BITS_ARB, int(FormatHelper::getStencilBits(this->info.depthFormat)),
			0
		};

		wglChoosePixelFormatARB(data->dc, pixelAttribs, NULL, 1, &pixelFormatId, &numFormats);

		if (!numFormats)
			oic::System::log()->fatal(errors::surface::contextError);

		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory(&pfd, sizeof(pfd));

		if (!DescribePixelFormat(data->dc, pixelFormatId, sizeof(pfd), &pfd))
			oic::System::log()->fatal(errors::surface::contextError);

		if (!SetPixelFormat(data->dc, pixelFormatId, &pfd))
			oic::System::log()->fatal(errors::surface::contextError);

		#ifndef NO_DEBUG
			constexpr int enableDebug = WGL_CONTEXT_DEBUG_BIT_ARB;
		#else
			constexpr int enableDebug{};
		#endif

		int contextAttribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, int(g.getData()->major),
			WGL_CONTEXT_MINOR_VERSION_ARB, int(g.getData()->minor),
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			WGL_CONTEXT_FLAGS_ARB, enableDebug,
			0
		};

		wglSwapIntervalEXT(swapchainInfo.useVSync);
		data->rc = wglCreateContextAttribsARB(data->dc, 0, contextAttribs);

		if (!data->rc || !wglMakeCurrent(data->dc, data->rc))
			oic::System::log()->fatal(errors::surface::contextError);

		//Enable debug callbacks

		#ifndef NO_DEBUG
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(glxDebugMessage, nullptr);
		#endif

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