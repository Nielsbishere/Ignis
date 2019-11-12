#include "system/system.hpp"
#include "system/log.hpp"
#include "graphics/gl_context.hpp"
#include "graphics/wgl_graphics.hpp"

namespace ignis {

	void Graphics::init() {

		data->platform = new Graphics::Data::Platform();

		//Create temporary windows

		WNDCLASSA tempWClass{};
		tempWClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		tempWClass.lpfnWndProc = DefWindowProcA;
		tempWClass.hInstance = GetModuleHandle(0);
		tempWClass.lpszClassName = "WGL";

		if (!RegisterClassA(&tempWClass))
			oic::System::log()->fatal("The OpenGL context couldn't be registered");

		HWND hwnd = data->platform->hwnd = CreateWindowExA(
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

		HDC dc = data->platform->dc = GetDC(hwnd);

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

		int pixelFormat = ChoosePixelFormat(dc, &pfd);

		if (!pixelFormat || !SetPixelFormat(dc, pixelFormat, &pfd))
			oic::System::log()->fatal("The OpenGL context couldn't be set");

		//Create render context

		HGLRC rc = data->platform->rc = wglCreateContext(dc);

		if (!rc || !wglMakeCurrent(dc, rc))
			oic::System::log()->fatal("The OpenGL context couldn't be made current");

		//Obtain the OpenGL version and max supported sample count

		glGetIntegerv(GL_MAX_SAMPLES, (GLint*)&data->maxSamples);
		glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&data->major);
		glGetIntegerv(GL_MINOR_VERSION, (GLint*)&data->minor);
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &data->maxAnistropy);

		if (!data->version(4, 6))
			oic::System::log()->fatal("OpenGL version not supported; >= 4.6 required");

		for (auto &elem : glFunctionNames) {

			*elem.second = wglGetProcAddress(elem.first.c_str());

			if (!elem.second)
				oic::System::log()->warn(String("GL Function not found ") + elem.first);
		}

		data->getContext();
	}

	void Graphics::release() {

		data->destroyContext();

		wglMakeCurrent(data->platform->dc, NULL);
		wglDeleteContext(data->platform->rc);
		ReleaseDC(data->platform->hwnd, data->platform->dc);
		DestroyWindow(data->platform->hwnd);
		UnregisterClassA("WGL", GetModuleHandleA(NULL));

		destroy(data->platform);
	}

	void Graphics::pause() {
		wglMakeCurrent(data->platform->dc, NULL);
	}

	void Graphics::resume() {
		wglMakeCurrent(data->platform->dc, data->platform->rc);
	}

}