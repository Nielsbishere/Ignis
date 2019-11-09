#include "graphics/surface/swapchain.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	void Swapchain::onResize(const Vec2u &size) {
		info.size = size;
	}

	void Swapchain::begin(const Vec4u &xywh) {
		glxBeginRenderPass(getGraphics().getData()->getContext(), xywh, info.size, 0);
	}

	void Swapchain::end() {

	}

}