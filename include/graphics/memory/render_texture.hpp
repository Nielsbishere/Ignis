#pragma once
#include "texture_object.hpp"

namespace ignis {

	class RenderTexture : public TextureObject {

	public:

		apimpl RenderTexture(Graphics &g, const String &name, const Info &info);

		TextureObjectType getTextureObjectType() const final override {
			return TextureObjectType::RENDER;
		}

		apimpl void onResize(const Vec2u32 &size);

	private:

		apimpl ~RenderTexture();

	};

}