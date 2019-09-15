#include "graphics/surface/swapchain.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	void Swapchain::onResize(const Vec2u &size) {
		info.size = size;
	}

	void Swapchain::begin(const Vec4u &xywh) {
		glBeginRenderPass(*getGraphics().getData(), xywh, info.size, 0);
	}

	void Swapchain::end() {

	}

}