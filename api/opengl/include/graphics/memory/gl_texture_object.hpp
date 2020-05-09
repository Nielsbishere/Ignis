#pragma once
#include "graphics/memory/texture.hpp"
#include "graphics/gl_graphics.hpp"
#include "graphics/shader/descriptors.hpp"

namespace ignis {

	struct TextureObject::Data {

		GLuint handle{};

		List<GLuint> framebuffer;		//Only if it's GPU_WRITABLE

		//Subresources
		List<std::pair<GPUSubresource::TextureRange, GLuint>> textureViews;

	};

}