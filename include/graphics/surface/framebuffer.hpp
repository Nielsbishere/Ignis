#pragma once
#include "surface.hpp"

namespace ignis {

	class Framebuffer : public Surface {

	public:

		apimpl struct Data;

		using Info = Surface::Info;

		apimpl Framebuffer(Graphics &g, const String &name, const Info &info);

		apimpl void onResize(const Vec2u32 &size) final override;

		apimpl void begin() final override;
		apimpl void end() final override;

		//TODO: isCompatible

		Data *getData() { return data; }

	private:
		
		apimpl ~Framebuffer();

		Data *data;
	};

}