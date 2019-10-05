#pragma once
#include "graphics/shader/pipeline.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	struct Pipeline::Data {
		List<GLuint> handles;
	};

}