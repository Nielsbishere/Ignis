#pragma once
#include "format.hpp"

namespace ignis {

	enum class GPUBufferType : u8 {
		UNIFORM, TEXTURE, VERTEX, INDEX,
		STORAGE, STRUCTURED,
		INDIRECT_DRAW, INDIRECT_DISPATCH
	};

	//This is a usage hint to how the GPU memory should behave:
	//Flags: 
	//& 1 = isShared		; is CPU accessible (!isShared = device local)
	//& 2 = isPreferred		; requires heap to be the same (!isPreferred = use specified heap)
	//& 4 = isGPUWritable	; can the resource be written to from the GPU?
	//& 8 = isCPUWritable	; can the CPU update this or is it just an initialization
	enum class GPUBufferUsage : u8 {

		LOCAL				= 0b00000000,
		SHARED				= 0b00000001,

		REQUIRE				= 0b00000000,
		PREFER				= 0b00000010,

		GPU_WRITE			= 0b00000100,
		CPU_WRITE			= 0b00001000
	};

	//This is the topology that should be assembled
	//Flags (& types):
	//& 3 = list, strip, fan, invalid
	//& 1 = isStrip
	//& 2 = isFan
	//(>> 2) & 3 = point, line, triangle, invalid
	//& 4 = isLine
	//& 8 = isTriangle
	//& 16 = isAdjacent
	enum class TopologyMode : u8 {

		POINT_LIST			= 0b00000,

		LINE_LIST			= 0b00100,
		LINE_STRIP			= 0b00101,

		TRIANGLE_LIST		= 0b01000,
		TRIANGLE_STRIP		= 0b01001,
		TRIANGLE_FAN		= 0b01010,

		LINE_LIST_ADJ		= 0b10100,
		LINE_STRIP_ADJ		= 0b10101,

		TRIANGLE_LIST_ADJ	= 0b11000,
		TRIANGLE_STRIP_ADJ	= 0b11001
	};

	//The shader stage type
	//& 0x80 = isExt
	//& 0x40 = isRaytracing
	//& 0x20 = isCompute
	enum class ShaderStage : u8 {

		VERTEX						= 0x00,
		GEOMETRY					= 0x01,
		TESSELATION_CONTROL			= 0x02,
		TESSELATION_EVALUATION		= 0x03,
		FRAGMENT					= 0x04,

		COMPUTE						= 0x20,

		TASK_FT						= 0x80,
		MESH_FT						= 0x81,

		RAYGEN_FT					= 0xC0,
		ANY_HIT_FT					= 0xC1,
		CLOSEST_HIT_FT				= 0xC2,
		MISS_FT						= 0xC3,
		INTERSECTION_FT				= 0xC4,
		CALLABLE_FT					= 0xC5
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

}