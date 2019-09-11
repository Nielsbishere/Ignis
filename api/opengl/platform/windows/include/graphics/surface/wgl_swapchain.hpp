#pragma once
#include "graphics/surface/swapchain.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace ignis {

	struct Swapchain::Data {
		HDC dc;
		HGLRC rc;
	};

}