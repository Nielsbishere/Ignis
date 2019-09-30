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
}