#include "graphics/memory/texture.hpp"
#include "graphics/shader/descriptors.hpp"
#include "utils/math.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {

	Texture::Info::Info(
		u32 x, GPUFormat format, GPUMemoryUsage usage,
		TextureMip mips, u32 layers
	) :
		Info(Vec3u{ x, 1, 1 }, format, usage, mips, layers) {

		textureType = 
			layers > 1 ? TextureType::TEXTURE_1D_ARRAY : TextureType::TEXTURE_1D;
	}

	Texture::Info::Info(
		Vec2u xy, GPUFormat format, GPUMemoryUsage usage,
		TextureMip mips, u32 layers
	) :
		Info(Vec3u{ xy[0], xy[1], 1 }, format, usage, mips, layers) {

		textureType = 
			layers > 1 ? TextureType::TEXTURE_2D_ARRAY : TextureType::TEXTURE_2D;
	}

	Texture::Info::Info(
		Vec3u xyz, GPUFormat format, GPUMemoryUsage usage,
		TextureMip mips, u32 layers
	): 
		dimensions(xyz), format(format), usage(usage), mips(mips), layers(layers),
		textureType(TextureType::TEXTURE_3D) {
	
		if (layers > 1)			//TODO: This is called for 1D and 2D arrays too!
			oic::System::log()->fatal("4D Textures aren't supported yet");

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
		u8 mipCount = u8(mips) & u8(TextureMip::PROPERTY_MIP_COUNT);

		if (mipCount)
			mipCount = oic::Math::min(mipCount, u8(biggestMip));
		else
			mipCount = biggestMip;

		this->mips = TextureMip(
			(u8(mips) & ~u8(TextureMip::PROPERTY_MIP_COUNT)) |
			mipCount
		);
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
		const RegisterLayout &reg, const GPUSubresource &
	) {
		return
			reg.type == ResourceType::TEXTURE &&
			reg.textureType == info.textureType &&
			(reg.textureFormat == GPUFormat::NONE || reg.textureFormat == info.format);

		//TODO: GPUSubresource validation
	}

}