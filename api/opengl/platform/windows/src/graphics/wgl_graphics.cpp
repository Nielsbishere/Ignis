#include "system/system.hpp"
#include "system/log.hpp"
#include "error/ignis.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	void Graphics::init() {

		//Create temporary windows

		WNDCLASSA tempWClass{};
		tempWClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		tempWClass.lpfnWndProc = DefWindowProcA;
		tempWClass.hInstance = GetModuleHandle(0);
		tempWClass.lpszClassName = "WGL";

		if (!RegisterClassA(&tempWClass))
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

		HDC dc = GetDC(tempWind);

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
			oic::System::log()->fatal(errors::surface::contextError);

		//Create render context

		HGLRC rc = wglCreateContext(dc);

		if (!rc || !wglMakeCurrent(dc, rc))
			oic::System::log()->fatal(errors::surface::contextError);

		//Obtain the OpenGL version and max supported sample count

		GLint samples, major, minor;
		glGetIntegerv(GL_MAX_SAMPLES, &samples);
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		data->major = u32(major);
		data->minor = u32(minor);
		data->maxSamples = u32(samples);

		if (!data->version(4, 6))
			oic::System::log()->fatal("OpenGL version not supported; >= 4.6 required");

		for (auto &elem : glFunctionNames) {

			*elem.second = wglGetProcAddress(elem.first.c_str());

			if (!elem.second)
				oic::System::log()->warn(String("GL Function not found ") + elem.first);
		}

		wglMakeCurrent(dc, NULL);
		wglDeleteContext(rc);
		ReleaseDC(tempWind, dc);
		DestroyWindow(tempWind);
		UnregisterClassA("WGL", GetModuleHandleA(NULL));
	}

}