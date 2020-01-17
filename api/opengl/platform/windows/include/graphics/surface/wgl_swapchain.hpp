#pragma once
#include "graphics/memory/swapchain.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	struct Swapchain::Data {
		HDC dc{};
		HGLRC rc{};
	};

}