#pragma once
#include "format.hpp"

namespace ignis {

	enum class GPUBufferType : u8 {
		UNIFORM, TEXTURE, VERTEX, INDEX,
		STORAGE_FT, STRUCTURED_FT,
		INDIRECT_DRAW_EXT, INDIRECT_DISPATCH_EXT
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
}