#pragma once
#include "graphics/shader/sampler.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	struct Sampler::Data {
		GLuint handle{};
	};

}