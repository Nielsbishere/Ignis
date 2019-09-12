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
			bool useVSync;

			Info(oic::ViewportInfo *vi, bool useVSync): vi(vi), useVSync(useVSync) {}
		};

		__impl struct Data;

		__impl Swapchain(Graphics &g, const Info &info);
		__impl ~Swapchain();

		__impl void onResize(const Vec2u &size) final override;

		__impl void begin(const Vec4u &xyzw) final override;
		__impl void end() final override;
		__impl void present();

		inline const Info &getSwapchainInfo() const { return swapchainInfo; }
		Data *getData() { return data; }

	private:

		Info swapchainInfo;
		Data *data;
	};

}