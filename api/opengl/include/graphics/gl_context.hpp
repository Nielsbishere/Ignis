#pragma once
#include "graphics/gl_graphics.hpp"

namespace ignis {

	class PrimitiveBuffer;
	class Framebuffer;

	//All states and state objects for a context

	struct GLContext {

		//VAOs; because they aren't shared

		HashMap<PrimitiveBuffer*, GLuint> vaos;
		List<PrimitiveBuffer*> deletedVaos;

		//States that have to be set with commands

		Framebuffer *currentFramebuffer{};
		PrimitiveBuffer *primitiveBuffer{};
		Pipeline *pipeline{};
		Descriptors *descriptors{};

		f32 depth{};
		u32 stencil{};

		CullMode cullMode{};
		WindMode windMode{};
		FillMode fillMode{};

		HashMap<GLenum, GLuint> bound;
		HashMap<u64, BoundRange> boundByBase;	//GLenum lower 32-bit, Base upper 32-bit

		Vec4u viewport{}, scissor{};

		Vec4f clearColor{};

		bool scissorEnable{};

	};

}