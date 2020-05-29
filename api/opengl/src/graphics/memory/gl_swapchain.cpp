#include "graphics/memory/swapchain.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	void Swapchain::onResize(const Vec2u32 &size) {
		getGraphics().wait();
		info.size = size.cast<Vec2u16>();
	}

}