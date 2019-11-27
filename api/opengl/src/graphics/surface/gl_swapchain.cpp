#include "graphics/surface/swapchain.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	void Swapchain::onResize(const Vec2u &size) {
		info.size = size;
	}

	void Swapchain::begin() {
		glxBeginRenderPass(getGraphics().getData()->getContext(), 0);
	}

	void Swapchain::end() {

	}

}