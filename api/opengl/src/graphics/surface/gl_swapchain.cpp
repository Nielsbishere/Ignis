#include "graphics/surface/swapchain.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	void Swapchain::onResize(const Vec2u &size) {
		info.size = size;
	}

	void Swapchain::begin(const Vec4u &xywh) {

		/*if (getGraphics().getData()->currentSurface != *this)
			glBindFramebuffer(0);*/

		Vec4u copy = xywh;

		if (!copy[2])
			copy[2] = getInfo().size[0] - copy[0];

		if (!copy[3])
			copy[3] = getInfo().size[1] - copy[1];

		if (getGraphics().getData()->xywh != xywh) {
			getGraphics().getData()->xywh = xywh;
			glViewport(xywh[0], xywh[1], xywh[2], xywh[3]);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

}