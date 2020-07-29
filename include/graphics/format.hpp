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

		R8 = 0x00,		RG8,	RGBA8 = 0x03,
		R16,			RG16,	RGBA16 = 0x07,

		R8s = 0x10,		RG8s,	RGBA8s = 0x13,
		R16s,			RG16s,	RGBA16s = 0x17,

		R8u = 0x20,		RG8u,	RGBA8u = 0x23,
		R16u,			RG16u,  RGBA16u = 0x27,
		R32u,			RG32u,	RGB32u, RGBA32u,
		R64u,			RG64u,	RGB64u, RGBA64u,

		R8i = 0x30,		RG8i,	RGBA8i = 0x33,
		R16i,			RG16i,  RGBA16i = 0x37,
		R32i,			RG32i,	RGB32i, RGBA32i,
		R64i,			RG64i,	RGB64i, RGBA64i,

		R16f = 0x74,	RG16f,	RGBA16f = 0x77,
		R32f,			RG32f,	RGB32f,	RGBA32f,
		R64f,			RG64f,	RGB64f, RGBA64f,

		sRGBA8 = 0x103,

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