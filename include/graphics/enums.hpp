#pragma once
#include "format.hpp"

namespace ignis {

	//A feature that's dependent on the current platform
	enum class Feature : u8 { 
		NONE, RAYTRACING, MESH_SHADERS, COUNT 
	};

	//An extension that's dependent on the current gpu
	enum class Extension : u8 { 
		NONE,
		CONDITIONAL_RENDERING,
		INDIRECT_COUNT,
		COUNT 
	};

	//0 = NONE
	//& 0xC = GPUBuffer (0), ShaderBuffer (4), DrawBuffer (8)
	enum class GPUBufferType : u8 {

		VERTEX				= 0x0,
		INDEX,

		UNIFORM				= 0x4,
		STORAGE,
		STRUCTURED,

		INDIRECT_DRAW		= 0x8,
		INDIRECT_DISPATCH,

		PROPERTY_TYPE		= 0xC
	};

	//This is a usage hint to how the GPU memory should behave:
	//Flags: 
	//& 1 = isShared		; is CPU accessible (!isShared = device local)
	//& 2 = isPreferred		; requires heap to be the same (!isPreferred = use specified heap)
	//& 4 = isGPUWritable	; can the resource be written to from the GPU?
	//& 8 = isCPUWritable	; can the CPU update this or is it just an initialization
	enum class GPUMemoryUsage : u8 {

		LOCAL				= 0x0,
		SHARED				= 0x1,

		REQUIRE				= 0x0,
		PREFER				= 0x2,

		GPU_WRITE			= 0x4,
		CPU_WRITE			= 0x8
	};

	//This is the topology that should be assembled
	//Flags (& types):
	//& 1 = list, strip
	//(>> 1) & 3 = point, line, triangle, invalid
	//& 4 = isLine
	//& 8 = isTriangle
	//& 16 = isAdjacent
	enum class TopologyMode : u8 {

		POINT_LIST				= 0x0,

		LINE_LIST				= 0x2,
		LINE_STRIP				= 0x3,

		TRIANGLE_LIST			= 0x4,
		TRIANGLE_STRIP			= 0x5,

		LINE_LIST_ADJ			= 0xA,
		LINE_STRIP_ADJ			= 0xB,

		TRIANGLE_LIST_ADJ		= 0xC,
		TRIANGLE_STRIP_ADJ		= 0xD,

		PROPERTY_IS_STRIP		= 0x1,
		PROPERTY_IS_LINE		= 0x2,
		PROPERTY_IS_TRIANGLE	= 0x4,
		PROPERTY_TYPE			= 0x6,
		PROPERTY_IS_ADJ			= 0x8
	};

	//The shader stage type
	//& 0x80 = isTechnique (ext or ft)
	//& 0x40 = isRaytracing
	//& 0x20 = isCompute
	//& 0x1F = typeId (1 << typeId = access flag)
	enum class ShaderStage : u8 {

		VERTEX						= 0x00,
		GEOMETRY					= 0x01,
		TESS_CTRL					= 0x02,
		TESS_EVAL					= 0x03,
		FRAGMENT					= 0x04,

		COMPUTE						= 0x25,

		TASK_EXT					= 0x86,
		MESH_EXT					= 0x87,

		RAYGEN_FT					= 0xC8,
		ANY_HIT_FT					= 0xC9,
		CLOSEST_HIT_FT				= 0xCA,
		MISS_FT						= 0xCB,
		INTERSECTION_FT				= 0xCC,
		CALLABLE_FT					= 0xCD,

		PROPERTY_TYPE				= 0x1F,		//1 << type = shader flag (32 bits)
		PROPERTY_IS_COMPUTE			= 0x20,
		PROPERTY_IS_RAYTRACING		= 0x40,
		PROPERTY_IS_TECHNIQUE		= 0x80
	};

	enum class ShaderAccess : u32 {

		VERTEX				= 1_u32 << (u8(ShaderStage::VERTEX) & u8(ShaderStage::PROPERTY_TYPE)),
		GEOMETRY			= 1_u32 << (u8(ShaderStage::GEOMETRY) & u8(ShaderStage::PROPERTY_TYPE)),
		TESS_CTRL			= 1_u32 << (u8(ShaderStage::TESS_CTRL) & u8(ShaderStage::PROPERTY_TYPE)),
		TESS_EVAL			= 1_u32 << (u8(ShaderStage::TESS_EVAL) & u8(ShaderStage::PROPERTY_TYPE)),
		FRAGMENT			= 1_u32 << (u8(ShaderStage::FRAGMENT) & u8(ShaderStage::PROPERTY_TYPE)),

		COMPUTE				= 1_u32 << (u8(ShaderStage::COMPUTE) & u8(ShaderStage::PROPERTY_TYPE)),

		TASK_EXT			= 1_u32 << (u8(ShaderStage::TASK_EXT) & u8(ShaderStage::PROPERTY_TYPE)),
		MESH_EXT			= 1_u32 << (u8(ShaderStage::MESH_EXT) & u8(ShaderStage::PROPERTY_TYPE)),

		RAYGEN_FT			= 1_u32 << (u8(ShaderStage::RAYGEN_FT) & u8(ShaderStage::PROPERTY_TYPE)),
		ANY_HIT_FT			= 1_u32 << (u8(ShaderStage::ANY_HIT_FT) & u8(ShaderStage::PROPERTY_TYPE)),
		CLOSEST_HIT_FT		= 1_u32 << (u8(ShaderStage::CLOSEST_HIT_FT) & u8(ShaderStage::PROPERTY_TYPE)),
		MISS_FT				= 1_u32 << (u8(ShaderStage::MISS_FT) & u8(ShaderStage::PROPERTY_TYPE)),
		INTERSECTION_FT		= 1_u32 << (u8(ShaderStage::INTERSECTION_FT) & u8(ShaderStage::PROPERTY_TYPE)),
		CALLABLE_FT			= 1_u32 << (u8(ShaderStage::CALLABLE_FT) & u8(ShaderStage::PROPERTY_TYPE))
	};

	//Rasterizer enums

	enum class FillMode : u8 {
		FILL, WIREFRAME
	};

	enum class WindMode : u8 {
		CCW, CW
	};

	enum class CullMode : u8 {
		NONE, FRONT, BACK
	};

	//Types of resources
	//Each one has its own localId counter

	enum class ResourceType : u8 {

		NONE = 0x00,

		CBUFFER,		//Uniform buffer only
		BUFFER,			//All other buffer types

		TEXTURE,
		SAMPLER,
		COMBINED_SAMPLER
	};

	//Types of textures and samplers

	//& 0x03 = dimension (CUBE, 1D, 2D, 3D)
	//& 0x04 = isMultisampled
	//& 0x08 = isArray
	enum class TextureType : u8 {

		TEXTURE_CUBE			= 0x0,
		TEXTURE_1D, 
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_MS				= 0x6,		//TODO: Figure out how this works with data and IXGI

		TEXTURE_CUBE_ARRAY		= 0x8, 
		TEXTURE_1D_ARRAY, 
		TEXTURE_2D_ARRAY,
		TEXTURE_MS_ARRAY		= 0xC,

		PROPERTY_DIMENSION		= 0x3,
		PROPERTY_IS_MS			= 0x4,
		PROPERTY_IS_ARRAY		= 0x8,	//1 << PROPERTY_IS_ARRAY_BIT
		PROPERTY_IS_ARRAY_BIT	= 0x3,

		ENUM_END				= 0x10
	};

	//& 0x03 = dimension (CUBE, 1D, 2D, 3D)
	//& 0x04 = isMultisampled
	//& 0x08 = isArray
	//& 0x0F = TextureType
	//& 0x10 = isCombinedSampler (can only be used on specified texture type)
	enum class SamplerType : u8 {

		SAMPLER					= 0x00,

		SAMPLER_CUBE			= 0x10,
		SAMPLER_1D, 
		SAMPLER_2D,
		SAMPLER_3D,
		SAMPLER_MS				= 0x16,

		SAMPLER_CUBE_ARRAY		= 0x18, 
		SAMPLER_1D_ARRAY, 
		SAMPLER_2D_ARRAY,
		SAMPLER_MS_ARRAY		= 0x1C,

		PROPERTY_DIMENSION		= 0x03,
		PROPERTY_IS_MS			= 0x04,
		PROPERTY_IS_ARRAY		= 0x08,
		PROPERTY_IS_COMBINED	= 0x10,
		PROPERTY_AS_TEXTURE		= 0x0F
	};

	//Minification filter
	//& 1 = useNearestFilter
	//& 2 = disableMips
	//& 4 = mipsUseNearestFilter
	enum class SamplerMin : u8 {

		LINEAR_MIPS					= 0x0, 
		LINEAR_MIPS_NEAREST, 

		LINEAR						= 0x2,
		NEAREST, 

		NEAREST_MIPS_LINEAR			= 0x4,
		NEAREST_MIPS,

		PROPERTY_USE_NEAREST		= 0x1,
		PROPERTY_DISABLE_MIPS		= 0x2,
		PROPERTY_MIPS_USE_NEAREST	= 0x4
	};

	//Magnification filter (& 1 = isNearest)
	enum class SamplerMag : u8 {
		LINEAR, NEAREST
	};

	//Sampler addressing mode
	//& 1 = doMirror
	//& 2 = useBorder
	//& 4 = doRepeat
	enum class SamplerMode : u8 {

		CLAMP_EDGE			= 0x0,
		MIRROR_CLAMP_EDGE,

		CLAMP_BORDER		= 0x2, 

		REPEAT				= 0x4,
		MIRROR_REPEAT,

		PROPERTY_MIRROR		= 0x1,
		PROPERTY_BORDER		= 0x2,
		PROPERTY_REPEAT		= 0x4
	};

}