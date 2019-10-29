#pragma once
#include "graphics/memory/texture.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	struct Texture::Data {
		GLuint handle{};
	};

}