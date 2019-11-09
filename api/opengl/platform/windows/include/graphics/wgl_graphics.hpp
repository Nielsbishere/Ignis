#pragma once
#include "graphics/gl_graphics.hpp"

namespace ignis {

	struct Graphics::Data::Platform {
		HDC dc{};
		HGLRC rc{};
		HWND hwnd{};
	};

}