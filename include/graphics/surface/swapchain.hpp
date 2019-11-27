#pragma once
#include "surface.hpp"

namespace oic {
	struct ViewportInfo;
}

namespace ignis {

	class Swapchain : public Surface {

	public:

		struct Info {

			oic::ViewportInfo *vi;
			DepthFormat format;
			bool useVSync;

			Info(oic::ViewportInfo *vi, bool useVSync, DepthFormat format): 
				vi(vi), useVSync(useVSync), format(format) {}
		};

		plimpl struct Data;

		plimpl Swapchain(Graphics &g, const String &name, const Info &info);

		apimpl void onResize(const Vec2u &size) final override;

		apimpl void begin() final override;
		apimpl void end() final override;
		plimpl void present();
		plimpl void bind();

		inline const Info &getSwapchainInfo() const { return swapchainInfo; }
		Data *getData() { return data; }

	private:

		plimpl ~Swapchain();

		Info swapchainInfo;
		Data *data;
	};

}