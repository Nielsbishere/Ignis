#include "graphics/memory/texture_object.hpp"
#include "graphics/shader/pipeline_layout.hpp"
#include "graphics/shader/descriptors.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "utils/math.hpp"

namespace ignis {

	TextureObject::Info::Info(
		u16 res, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u16 layers
	) :
		Info(
			TextureType(u8(TextureType::TEXTURE_1D) | ((layers > 1) << u8(TextureType::PROPERTY_IS_ARRAY_BIT))), 
			Vec3u16(res, 1, 1), format, usage, mips, layers, 1, true
		) { }

	TextureObject::Info::Info(
		const Vec2u16 &res, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u16 layers
	) :
		Info(
			TextureType(u8(TextureType::TEXTURE_2D) | ((layers > 1) << u8(TextureType::PROPERTY_IS_ARRAY_BIT))), 
			Vec3u16(res.x, res.y, 1), format, usage, mips, layers, 1, true
		) { }

	TextureObject::Info::Info(
		const Vec2u16 &res, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u16 layers, u8 samples, bool useFixedSampleLocations
	) :
		Info(
			TextureType(
				u8(samples > 1 ? TextureType::TEXTURE_MS : TextureType::TEXTURE_2D) | 
				((layers > 1) << u8(TextureType::PROPERTY_IS_ARRAY_BIT))
			), 
			Vec3u16(res.x, res.y, 1), format, usage, mips, layers, samples, useFixedSampleLocations
		) { }

	TextureObject::Info::Info(
		TextureType textureType, GPUFormat format, GPUMemoryUsage usage,
		u8 mipCount, u16 layers, u8 samples, bool useFixedSampleLocations
	) :
		dimensions(Vec3u16{}), format(format), usage(usage),
		mips(mipCount), layers(layers), textureType(textureType),
		samples(samples), useFixedSampleLocations(useFixedSampleLocations) {}

	TextureObject::Info::Info(
		TextureType textureType,
		const Vec3u16 &xyz, GPUFormat format, GPUMemoryUsage usage,
		u8 mipCount, u16 layers, u8 samples, bool useFixedSampleLocations
	):
		dimensions(xyz), format(format), usage(usage),
		mips(mipCount), layers(layers), textureType(textureType),
		samples(samples), useFixedSampleLocations(useFixedSampleLocations) {

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

	TextureObject::Info::Info(
		const Vec3u16 &res, GPUFormat format, GPUMemoryUsage usage,
		u8 mipCount
	): 
		Info(TextureType::TEXTURE_3D, res, format, usage, mipCount, 1, 1, true) { }

	bool TextureObject::isCompatible(
		const RegisterLayout &reg, const GPUSubresource &sub
	) const {
		return
			reg.type == ResourceType::TEXTURE &&
			reg.textureType == info.textureType &&
			(reg.textureFormat == GPUFormat::NONE || reg.textureFormat == info.format) &&
			validSubresource(sub);
	}

	bool TextureObject::validSubresource(const GPUSubresource &res, bool isSampler) const {

		auto &tex = isSampler ? (const GPUSubresource::TextureRange&)res.samplerData : res.textureRange;

		return 
			usz(tex.minLayer) + tex.layerCount <= info.layers && 
			usz(tex.minLevel) + tex.levelCount <= info.mips &&
			isValidSubType(tex.subType);
	}

	bool TextureObject::isValidSubType(const TextureType type) const {

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