#pragma once
#include "types/enum.hpp"

namespace ignis {

	//& 0x1						= hasStencil
	//((& 0x6) << 2) + 16		= channelDepth
	//& 0x10					= isFloatingPoint
	//& 0x20					= isAuto
	//& 0x40					= isNone
	enum class DepthFormat : u8 {

		D16	= 0x00,
		D24 = 0x02, D24_S8,
		D32 = 0x04,
		D32F = 0x14, D32F_S8,

		AUTO_DEPTH = 0x20, AUTO_DEPTH_STENCIL,
		NONE = 0x40
	};

	//1 nibble (0x0-0xF)
	//& 1			= isSigned
	//& 2			= isUnnormalized
	//& 4			= isFloatingPoint
	enum class GPUFormatType : u8 {

		UNORM,	SNORM,
		UINT,	SINT,

		FLOAT = 7,

		PROPERTY_IS_SIGNED = 1,
		PROPERTY_IS_UNNORMALIZED = 2,
		PROPERTY_IS_FLOATING_POINT = 4
	};

	//(& 3) + 1			= channel count
	//1 << ((>> 2) & 3)	= channel stride
	//(>> 4) & 0xF		= type
	//& 0x10			= isSigned
	//& 0x20			= isUnnormalized
	//& 0x40			= isFloatingPoint
	//& 0x100			= isSRGB
	//& 0x200			= isNone
	oicExposedEnum(GPUFormat, u16,

		r8 = 0x00,		rg8,	rgba8 = 0x03,
		r16,			rg16,	rgba16 = 0x07,

		r8s = 0x10,		rg8s,	rgba8s = 0x13,
		r16s,			rg16s,	rgba16s = 0x17,

		r8u = 0x20,		rg8u,	rgba8u = 0x23,
		r16u,			rg16u,  rgba16u = 0x27,
		r32u,			rg32u,	rgb32u, rgba32u,
		r64u,			rg64u,	rgb64u, rgba64u,

		r8i = 0x30,		rg8i,	rgba8i = 0x33,
		r16i,			rg16i,  rgba16i = 0x37,
		r32i,			rg32i,	rgb32i, rgba32i,
		r64i,			rg64i,	rgb64i, rgba64i,

		r16f = 0x74,	rg16f,	rgba16f = 0x77,
		r32f,			rg32f,	rgb32f,	rgba32f,
		r64f,			rg64f,	rgb64f, rgba64f,

		srgba8 = 0x103,

		NONE = 0x200
	);

	struct FormatHelper {

		static constexpr bool isFloatingPoint(DepthFormat df);
		static constexpr bool isFloatingPoint(GPUFormat gf);
		static constexpr bool isFloatingPoint(GPUFormatType gft);

		static constexpr bool hasStencil(DepthFormat df);
		static constexpr bool isSigned(GPUFormat gf);
		static constexpr bool isSigned(GPUFormatType gft);

		static constexpr bool isAuto(DepthFormat df);
		static constexpr bool isUnnormalized(GPUFormat gf);
		static constexpr bool isUnnormalized(GPUFormatType gft);

		static constexpr bool isNone(DepthFormat df);
		static constexpr bool isNone(GPUFormat gf);
		static constexpr GPUFormatType getType(GPUFormat gf);

		static constexpr usz getDepthBits(DepthFormat df);
		static constexpr usz getStencilBits(DepthFormat df);
		static constexpr usz getDepthBytes(DepthFormat df);
		static constexpr usz getStencilBytes(DepthFormat df);

		static constexpr bool isSRGB(GPUFormat gf);
		static constexpr bool flipRGB(GPUFormat gf);

		static constexpr bool isInt(GPUFormat gf);
		static constexpr bool isInt(GPUFormatType gft);

		static constexpr usz getStrideBits(GPUFormat gf);
		static constexpr usz getStrideBytes(GPUFormat gf);
		static constexpr usz getChannelCount(GPUFormat gf);
		static constexpr usz getSizeBits(GPUFormat gf);
		static constexpr usz getSizeBytes(GPUFormat gf);
	};

	constexpr bool FormatHelper::isFloatingPoint(DepthFormat df) { return u8(df) & 0x10; }
	constexpr bool FormatHelper::isFloatingPoint(GPUFormat gf) { return u8(gf.value) & 0x40; }
	constexpr bool FormatHelper::isFloatingPoint(GPUFormatType gft) { return u8(gft) & 0x4; }

	constexpr bool FormatHelper::hasStencil(DepthFormat df) { return u8(df) & 0x1; }
	constexpr bool FormatHelper::isSigned(GPUFormat gf) { return u8(gf.value) & 0x10; }
	constexpr bool FormatHelper::isSigned(GPUFormatType gft) { return u8(gft) & 0x1; }

	constexpr bool FormatHelper::isAuto(DepthFormat df) { return u8(df) & 0x20; }
	constexpr bool FormatHelper::isUnnormalized(GPUFormat gf) { return u8(gf.value) & 0x20; }
	constexpr bool FormatHelper::isUnnormalized(GPUFormatType gft) { return u8(gft) & 0x2; }

	constexpr bool FormatHelper::isNone(DepthFormat df) { return u8(df) & 0x40; }
	constexpr bool FormatHelper::isNone(GPUFormat gf) { return u16(gf.value) & 0x400; }
	constexpr GPUFormatType FormatHelper::getType(GPUFormat gf) { return GPUFormatType(u8(gf.value) >> 4); }

	constexpr usz FormatHelper::getDepthBits(DepthFormat df) { return df == DepthFormat::NONE ? 0 : 16_usz + (usz(u8(df) & 0x6) << 2); }
	constexpr usz FormatHelper::getDepthBytes(DepthFormat df) { return df == DepthFormat::NONE ? 0 : 2_usz + (usz(u8(df) & 0x6) >> 1); }
	constexpr usz FormatHelper::getStencilBytes(DepthFormat df) { return hasStencil(df); }
	constexpr usz FormatHelper::getStencilBits(DepthFormat df) { return getStencilBytes(df) << 3; }

	constexpr bool FormatHelper::isSRGB(GPUFormat gf) { return u16(gf.value) & 0x100; }
	constexpr bool FormatHelper::flipRGB(GPUFormat gf) { return u16(gf.value) & 0x200; }

	constexpr bool FormatHelper::isInt(GPUFormat gf) { return isUnnormalized(gf) && !isFloatingPoint(gf); }
	constexpr bool FormatHelper::isInt(GPUFormatType gft) { return isUnnormalized(gft) && !isFloatingPoint(gft); }

	constexpr usz FormatHelper::getStrideBits(GPUFormat gf) { return 8_usz << ((u8(gf.value) >> 2) & 3); }
	constexpr usz FormatHelper::getStrideBytes(GPUFormat gf) { return 1_usz << ((u8(gf.value) >> 2) & 3); }
	constexpr usz FormatHelper::getChannelCount(GPUFormat gf) { return 1_usz + (u8(gf.value) & 3); }
	constexpr usz FormatHelper::getSizeBits(GPUFormat gf) { return getStrideBits(gf) * getChannelCount(gf); }
	constexpr usz FormatHelper::getSizeBytes(GPUFormat gf) { return getStrideBytes(gf) * getChannelCount(gf); }
}

oicEnumHash(ignis::GPUFormat);