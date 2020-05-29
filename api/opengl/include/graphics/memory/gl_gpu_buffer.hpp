#pragma once
#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	struct GPUBuffer::Data {
		volatile u8 *unmapped{};
		GLuint handle{};
	};
}