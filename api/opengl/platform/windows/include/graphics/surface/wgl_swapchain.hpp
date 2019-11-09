#pragma once
#include "graphics/surface/swapchain.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	struct Swapchain::Data {
		HDC dc;
		HGLRC rc;
	};

}