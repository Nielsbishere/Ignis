#pragma once
#include "graphics/gl_graphics.hpp"
#include "shader/gl_pipeline.hpp"
#include "graphics/command/commands.hpp"

namespace ignis {

	class PrimitiveBuffer;
	class Framebuffer;

	//All states and state objects for a context

	struct GLContext {

		//Large objects

		Rasterizer currRaster{};
		BlendState currBlend{};
		DepthStencil currDepth{};

		Vec2u32 viewportSize{}, scissorSize{};
		Vec2i32 viewportOff{}, scissorOff{};

		cmd::SetClearColor clearColor{};

		//VAOs; because they aren't shared

		HashMap<PrimitiveBuffer*, GLuint> vaos;
		List<PrimitiveBuffer*> deletedVaos;

		//Current bound objects

		HashMap<GLenum, GLuint> bound;
		HashMap<u64, BoundRange> boundByBase;	//GLenum lower 32-bit, Base upper 32-bit

		//States that have to be set with commands

		Framebuffer *currentFramebuffer{};
		PrimitiveBuffer *primitiveBuffer{};
		Pipeline *pipeline{};
		Descriptors *descriptors{};

		u64 frameId {};
		f32 depth = 1, minSampleShading{};
		u32 stencil {};

		bool enableScissor{};
		bool enableMinSampleShading{};

		GLContext() {
			currRaster.cull = CullMode::NONE;
		}

	};

}