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

		if (!mips || mips > biggestMip)
			oic::System::log()->fatal("Texture created with too many or no mips!");
	}

	bool Texture::Info::init(const List<Buffer> &b) {

		if (b.size() != usz(mips)) {
			oic::System::log()->error("Texture requires all init data (mips and layers)");
			return false;
		}

		Vec3u res = dimensions;

		for (usz m = 0; m < mips; ++m) {

			usz size =
				FormatHelper::getSizeBytes(format) * 
				res[0] * res[1] * res[2] * layers;

			if (b[m].size() != size || !size) {
				oic::System::log()->error("Invalid texture size");
				return false;
			}

			//TODO: Use Vec3f

			f64 x = res[0], y = res[1], z = res[2];
			x /= 2; y /= 2; z /= 2;

			res = Vec3u{ u32(oic::Math::ceil(x)), u32(oic::Math::ceil(y)), u32(oic::Math::ceil(z)) };
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