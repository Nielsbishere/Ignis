#pragma once
#include "graphics/memory/framebuffer.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	//Framebuffer definition

	struct Framebuffer::Data {
		GLuint index{};
	};

}