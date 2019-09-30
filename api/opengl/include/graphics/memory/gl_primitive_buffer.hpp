#pragma once
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	struct PrimitiveBuffer::Data {
		GLuint handle{};
	};
}