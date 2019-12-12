#pragma once
#include "graphics/gl_graphics.hpp"
#include "shader/gl_pipeline.hpp"
#include "graphics/command/commands.hpp"

namespace ignis {

	class PrimitiveBuffer;
	class Framebuffer;

	//All states and state objects for a context

	struct GLContext {

		using BlendState = Pipeline::BlendState;
		using Rasterizer = Pipeline::Rasterizer;

		//Large objects

		Rasterizer currRaster{};
		BlendState currBlend{};

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

		f32 depth{}, minSampleShading{};
		u32 stencil{}, frameId{};

		bool enableScissor{};
		bool enableMinSampleShading{};

	};

}