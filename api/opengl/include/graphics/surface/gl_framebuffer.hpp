#pragma once
#include "graphics/surface/framebuffer.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	//Framebuffer definition

	struct Framebuffer::Data {
		GLuint index {};
		GLuint depth {};
		List<GLuint> renderTextures;
	};

}