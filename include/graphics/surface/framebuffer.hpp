#pragma once
#include "surface.hpp"

namespace ignis {

	struct Framebuffer : public Surface {

	public:

		__impl struct Data;

		__impl Framebuffer(Graphics &g, const Info &info);
		__impl ~Framebuffer();

		__impl void onResize(const Vec2u &size) final override;

		__impl void begin() final override;
		__impl void end() final override;

		Data *getData() { return data; }

	private:

		Data *data;
	};

}