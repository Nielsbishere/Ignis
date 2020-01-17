#pragma once
#include "texture_object.hpp"

namespace ignis {

	class DepthTexture : public TextureObject {

	public:

		struct Info : public TextureObject::Info {

			bool storeData;
			DepthFormat format;

			Info(
				DepthFormat format, bool storeData,
				GPUMemoryUsage usage, u8 mips, u16 layers
			) :
				TextureObject::Info(
					layers > 1 ? TextureType::TEXTURE_2D_ARRAY : TextureType::TEXTURE_2D,
					GPUFormat::NONE,
					usage, mips, layers,
					1, true
				),
				storeData(storeData), format(format) {}

			Info(
				DepthFormat format, bool storeData, GPUMemoryUsage usage,
				u8 mips, u16 layers, u8 samples, bool useFixedSampleLocations
			) :
				TextureObject::Info(
					samples > 1 ? (
						layers > 1 ? TextureType::TEXTURE_MS_ARRAY : TextureType::TEXTURE_MS
					) : (
						layers > 1 ? TextureType::TEXTURE_2D_ARRAY : TextureType::TEXTURE_2D
					),
					GPUFormat::NONE,
					usage, mips, layers,
					samples, useFixedSampleLocations
				),
				storeData(storeData), format(format) {}

		};

		apimpl DepthTexture(Graphics &g, const String &name, const Info &info);

		TextureObjectType getTextureObjectType() const final override {
			return TextureObjectType::DEPTH;
		}

		apimpl void onResize(const Vec2u32 &size);

		bool isCompatible(
			const RegisterLayout &reg, const GPUSubresource &resource
		) const override {

			if (storeData) return TextureObject::isCompatible(reg, resource);
			return false;
		}

		inline DepthFormat getFormat() const { return format; }

	private:

		apimpl ~DepthTexture();

		DepthFormat format;
		bool storeData{};

	};

}