#pragma once
#include "graphics/gl_graphics.hpp"

namespace ignis {

	class PrimitiveBuffer;
	class Framebuffer;

	//Since every GL context has "setting objects" that don't share properly
	//We need to generate some objects per context that don't carry over.
	//These have to be removed whenever the object is removed from Graphics.
	//gl_graphics keeps track of all contexts and pending deletions and additions.

	struct GLContext {

		HashMap<PrimitiveBuffer*, GLuint> vaos;
		List<PrimitiveBuffer*> deletedVaos;

		//TODO: We should maintain all Graphics::Data's states in here

	};

}