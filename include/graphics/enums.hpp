#pragma once
#include "format.hpp"

namespace ignis {

	//0 = NONE
	//& 0xC = GPUBuffer (0), ShaderBuffer (4), DrawBuffer (8)
	enum class GPUBufferType : u8 {

		NONE = 0x0,		//Used for specifying a buffer without a type
		VERTEX,
		INDEX,

		UNIFORM	= 0x4,
		STORAGE,
		STRUCTURED,

		INDIRECT_DRAW = 0x8,
		INDIRECT_DISPATCH,

		PROPERTY_TYPE = 0xC
	};

	//This is a usage hint to how the GPU memory should behave:
	//Flags: 
	//& 1 = isShared		; is CPU accessible (!isShared = device local)
	//& 2 = isPreferred		; requires heap to be the same (!isPreferred = use specified heap)
	//& 4 = isGPUWritable	; can the resource be written to from the GPU?
	//& 8 = isCPUWritable	; can the CPU update this or is it just an initialization
	enum class GPUMemoryUsage : u8 {

		LOCAL				= 0b00000000,
		SHARED				= 0b00000001,

		REQUIRE				= 0b00000000,
		PREFER				= 0b00000010,

		GPU_WRITE			= 0b00000100,
		CPU_WRITE			= 0b00001000
	};

	//This is the topology that should be assembled
	//Flags (& types):
	//& 1 = list, strip
	//(>> 1) & 3 = point, line, triangle, invalid
	//& 4 = isLine
	//& 8 = isTriangle
	//& 16 = isAdjacent
	enum class TopologyMode : u8 {

		POINT_LIST				= 0b0000,

		LINE_LIST				= 0b0010,
		LINE_STRIP				= 0b0011,

		TRIANGLE_LIST			= 0b0100,
		TRIANGLE_STRIP			= 0b0101,

		LINE_LIST_ADJ			= 0b1010,
		LINE_STRIP_ADJ			= 0b1011,

		TRIANGLE_LIST_ADJ		= 0b1100,
		TRIANGLE_STRIP_ADJ		= 0b1101,

		PROPERTY_IS_STRIP		= 0b0001,
		PROPERTY_IS_LINE		= 0b0010,
		PROPERTY_IS_TRIANGLE	= 0b0100,
		PROPERTY_TYPE			= 0b0110,
		PROPERTY_IS_ADJ			= 0b1000
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

	enum ShaderAccess : u32 {

		ACCESS_VERTEX				= 1_u32 << (u8(ShaderStage::VERTEX) & u8(ShaderStage::PROPERTY_TYPE)),
		ACCESS_GEOMETRY				= 1_u32 << (u8(ShaderStage::GEOMETRY) & u8(ShaderStage::PROPERTY_TYPE)),
		ACCESS_TESS_CTRL			= 1_u32 << (u8(ShaderStage::TESS_CTRL) & u8(ShaderStage::PROPERTY_TYPE)),
		ACCESS_TESS_EVAL			= 1_u32 << (u8(ShaderStage::TESS_EVAL) & u8(ShaderStage::PROPERTY_TYPE)),
		ACCESS_FRAGMENT				= 1_u32 << (u8(ShaderStage::FRAGMENT) & u8(ShaderStage::PROPERTY_TYPE)),

		ACCESS_COMPUTE				= 1_u32 << (u8(ShaderStage::COMPUTE) & u8(ShaderStage::PROPERTY_TYPE)),

		ACCESS_TASK_EXT				= 1_u32 << (u8(ShaderStage::TASK_EXT) & u8(ShaderStage::PROPERTY_TYPE)),
		ACCESS_MESH_EXT				= 1_u32 << (u8(ShaderStage::MESH_EXT) & u8(ShaderStage::PROPERTY_TYPE)),

		ACCESS_RAYGEN_FT			= 1_u32 << (u8(ShaderStage::RAYGEN_FT) & u8(ShaderStage::PROPERTY_TYPE)),
		ACCESS_ANY_HIT_FT			= 1_u32 << (u8(ShaderStage::ANY_HIT_FT) & u8(ShaderStage::PROPERTY_TYPE)),
		ACCESS_CLOSEST_HIT_FT		= 1_u32 << (u8(ShaderStage::CLOSEST_HIT_FT) & u8(ShaderStage::PROPERTY_TYPE)),
		ACCESS_MISS_FT				= 1_u32 << (u8(ShaderStage::MISS_FT) & u8(ShaderStage::PROPERTY_TYPE)),
		ACCESS_INTERSECTION_FT		= 1_u32 << (u8(ShaderStage::INTERSECTION_FT) & u8(ShaderStage::PROPERTY_TYPE)),
		ACCESS_CALLABLE_FT			= 1_u32 << (u8(ShaderStage::CALLABLE_FT) & u8(ShaderStage::PROPERTY_TYPE))
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
		NONE,
		CBUFFER,		//Uniform buffer only
		BUFFER,			//All other buffer types
		TEXTURE,
		SAMPLER
	};

	//Types of textures and samplers

	//& 0x03 = dimension (CUBE, 1D, 2D, 3D)
	//& 0x04 = isMultisampled
	//& 0x08 = isArray
	enum class TextureType : u8 {

		TEXTURE_CUBE		= 0x00,
		TEXTURE_1D, 
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_MS			= 0x06,

		TEXTURE_CUBE_ARRAY	= 0x08, 
		TEXTURE_1D_ARRAY, 
		TEXTURE_2D_ARRAY,
		TEXTURE_MS_ARRAY	= 0x0C,

		PROPERTY_DIMENSION		= 0x03,
		PROPERTY_IS_MS			= 0x04,
		PROPERTY_IS_ARRAY		= 0x08
	};

	//How the mips are generated and how many are used
	//& 0x80 = isNearest
	//& 0x1F = mipCount
	//!(& 0x1F) = isAuto (automaticaly determines mips)
	//If mipCount is set to non zero, it will use the mipCount

	enum class TextureMip : u8 {

		AUTO = 0x00,			//N mipmaps (based on resolution); linear by default
		NONE = 0x01,
		LINEAR = 0x00,
		NEAREST = 0x80,

		PROPERTY_IS_NEAREST = 0x80,
		PROPERTY_MIP_COUNT = 0x1F
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

}