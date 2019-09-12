#include "graphics/surface/swapchain.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	void Swapchain::onResize(const Vec2u &size) {
		info.size = size;
	}

	void Swapchain::begin(const Vec4u &xywh) {

		/*if (getGraphics().getData()->currentSurface != *this)
			glBindFramebuffer(0);*/

		Vec4u sc = xywh, vp = { 0, 0, info.size[0], info.size[1] };

		if (!sc[2])
			sc[2] = vp[2] - sc[0];

		if (!sc[3])
			sc[3] = vp[3] - sc[1];

		Graphics::Data *gdata = getGraphics().getData();

		if (gdata->viewport != vp) {
			gdata->viewport = vp;
			glViewport(vp[0], vp[1], vp[2], vp[3]);
		}

		if (sc == vp) {
			if (gdata->scissorEnable) {
				glDisable(GL_SCISSOR_TEST);
				gdata->scissorEnable = false;
			}
		} else if (!gdata->scissorEnable) {
			glEnable(GL_SCISSOR_TEST);
			gdata->scissorEnable = true;
		}
		
		if (gdata->scissor != sc) {
			gdata->scissor = sc;
			glScissor(sc[0], sc[1], sc[2], sc[3]);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Swapchain::end() {

	}

}