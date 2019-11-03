#include "graphics/memory/texture.hpp"
#include "graphics/shader/descriptors.hpp"
#include "utils/math.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {

	Texture::Info::Info(
		u32 x, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) :
		Info(
			TextureType(u8(TextureType::TEXTURE_1D) | ((layers > 1) << u8(TextureType::PROPERTY_IS_ARRAY_BIT))), 
			Vec3u{ x, 1, 1 }, format, usage, mips, layers
		) { }

	Texture::Info::Info(
		Vec2u xy, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) :
		Info(
			TextureType(u8(TextureType::TEXTURE_2D) | ((layers > 1) << u8(TextureType::PROPERTY_IS_ARRAY_BIT))), 
			Vec3u{ xy[0], xy[1], 1 }, format, usage, mips, layers
		) { }

	Texture::Info::Info(
		Vec3u xyz, GPUFormat format, GPUMemoryUsage usage,
		u8 mipCount
	): 
		Info(TextureType::TEXTURE_3D, xyz, format, usage, mipCount, 1) { }

	Texture::Info::Info(
		TextureType textureType,
		Vec3u xyz, GPUFormat format, GPUMemoryUsage usage,
		u8 mipCount, u32 layers
	): 
		dimensions(xyz), format(format), usage(usage), mips(mipCount), layers(layers),
		textureType(textureType) {

		//Automatically determine mips

		u32 biggestRes =
			oic::Math::max(
				oic::Math::max(
					dimensions[0],
					dimensions[1]
				),
				dimensions[2]
			);

		u8 biggestMip = u8(oic::Math::ceil(oic::Math::log2<f64>(biggestRes)) + 1);

		if (mips)
			mips = oic::Math::min(mips, biggestMip);
		else
			mips = biggestMip;
	}

	bool Texture::Info::init(const Buffer &b) {

		usz size =
			FormatHelper::getSizeBytes(format) * 
			dimensions[0] * dimensions[1] * dimensions[2];

		if (b.size() != size || !size) {
			oic::System::log()->error("Invalid texture size");
			return false;
		}

		initData = b;
		return true;
	}
	
	bool Texture::isCompatible(
		const RegisterLayout &reg, const GPUSubresource &sub
	) const {
		return
			reg.type == ResourceType::TEXTURE &&
			reg.textureType == info.textureType &&
			(reg.textureFormat == GPUFormat::NONE || reg.textureFormat == info.format) &&
			validSubresource(sub);
	}

	bool Texture::validSubresource(const GPUSubresource &res, bool isSampler) const {
		auto &tex = isSampler ? (const GPUSubresource::TextureRange&)res.samplerData : res.textureRange;
		return usz(tex.minLayer) + tex.layerCount <= info.layers && usz(tex.minLevel) + tex.levelCount <= info.mips;
	}

}