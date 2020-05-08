#include "graphics/memory/swapchain.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	void Swapchain::onResize(const Vec2u32 &size) {
		info.size = size.cast<Vec2u16>();
	}

	void Swapchain::begin() {
		glxBeginRenderPass(getGraphics().getData()->getContext(), {}, 0);
	}

	void Swapchain::end() {

	}

}