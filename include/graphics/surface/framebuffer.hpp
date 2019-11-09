#pragma once
#include "surface.hpp"

namespace ignis {

	class Framebuffer : public Surface {

	public:

		apimpl struct Data;

		apimpl Framebuffer(Graphics &g, const String &name, const Info &info);
		apimpl ~Framebuffer();

		apimpl void onResize(const Vec2u &size) final override;

		apimpl void begin(const Vec4u &area) final override;
		apimpl void end() final override;

		//TODO:
		/*bool isCompatible(
			const RegisterLayout &reg, const GPUSubresource &resource
		) const final override;*/

		Data *getData() { return data; }

	private:

		Data *data;
	};

}