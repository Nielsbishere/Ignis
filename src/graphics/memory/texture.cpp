#include "graphics/memory/texture.hpp"
#include "graphics/shader/descriptors.hpp"
#include "utils/math.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {

	Texture::Info::Info(
		u32 res, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) :
		Info(
			TextureType(u8(TextureType::TEXTURE_1D) | ((layers > 1) << u8(TextureType::PROPERTY_IS_ARRAY_BIT))), 
			Vec3u32(res, 1, 1), format, usage, mips, layers
		) { }

	Texture::Info::Info(
		const Vec2u32 &res, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) :
		Info(
			TextureType(u8(TextureType::TEXTURE_2D) | ((layers > 1) << u8(TextureType::PROPERTY_IS_ARRAY_BIT))), 
			Vec3u32(res.x, res.y, 1), format, usage, mips, layers
		) { }

	Texture::Info::Info(
		const Vec3u32 &res, GPUFormat format, GPUMemoryUsage usage,
		u8 mipCount
	): 
		Info(TextureType::TEXTURE_3D, res, format, usage, mipCount, 1) { }

	Texture::Info::Info(
		TextureType textureType,
		const Vec3u32 &xyz, GPUFormat format, GPUMemoryUsage usage,
		u8 mipCount, u32 layers
	): 
		dimensions(xyz), layers(layers), format(format), usage(usage),
		mips(mipCount), textureType(textureType) {

		if(!layers || !xyz.x || !xyz.y || !xyz.z)
			oic::System::log()->fatal("Texture created with invalid dimensions!");

		//Automatically determine mips

		u32 biggestRes =
			oic::Math::max(
				oic::Math::max(
					dimensions.x,
					dimensions.y
				),
				dimensions.z
			);

		u8 biggestMip = u8(oic::Math::ceil(oic::Math::log2<f64>(biggestRes)) + 1);

		if (!mips || mips > biggestMip)
			oic::System::log()->fatal("Texture created with too many or no mips!");
	}

	bool Texture::Info::init(const List<Buffer> &b) {

		if (b.size() != usz(mips)) {
			oic::System::log()->error("Texture requires all init data (mips and layers)");
			return false;
		}

		Vec3u32 res = dimensions;

		for (usz m = 0; m < mips; ++m) {

			usz size =
				FormatHelper::getSizeBytes(format) * 
				res.x * res.y * res.z * layers;

			if (b[m].size() != size || !size) {
				oic::System::log()->error("Invalid texture size");
				return false;
			}

			res = (res.cast<Vec3f32>() / 2).ceil().cast<Vec3u32>();
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

		return 
			usz(tex.minLayer) + tex.layerCount <= info.layers && 
			usz(tex.minLevel) + tex.levelCount <= info.mips &&
			isValidSubType(tex.subType);
	}

	bool Texture::isValidSubType(const TextureType type) const {

		if (info.textureType == TextureType::TEXTURE_CUBE)
			return
				type == TextureType::TEXTURE_2D || type == TextureType::TEXTURE_2D_ARRAY ||
				type == TextureType::TEXTURE_CUBE;

		if (info.textureType == TextureType::TEXTURE_CUBE_ARRAY)
			return 
				type == TextureType::TEXTURE_2D || type == TextureType::TEXTURE_2D_ARRAY ||
				type == TextureType::TEXTURE_CUBE || type == TextureType::TEXTURE_CUBE_ARRAY;

		return info.textureType == type || TextureType(u8(info.textureType) & ~u8(TextureType::PROPERTY_IS_ARRAY)) == type;
	}

}