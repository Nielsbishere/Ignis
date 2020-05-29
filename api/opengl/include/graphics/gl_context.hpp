#pragma once
#include "graphics/gl_graphics.hpp"
#include "shader/gl_pipeline.hpp"
#include "graphics/command/commands.hpp"

namespace ignis {

	class PrimitiveBuffer;
	class Framebuffer;

	//All states and state objects for a context

	struct GLContext {

		//Objects that have to be set in OpenGL

		struct Bound {

			cmd::SetViewport viewport;
			cmd::SetScissor scissor;

			Framebuffer *framebuffer{};
			PrimitiveBuffer *primitiveBuffer{};
			Descriptors *descriptors{};
			Pipeline *pipeline{};

		} bound, boundApi;

		//VAOs; because they aren't shared (by object id)

		HashMap<GPUObjectId, GLuint> vaos;

		//Constants that aren't intermediate states

		HashMap<GLenum, GPUObjectId> boundObjects;
		HashMap<u64, BoundRange> boundByBaseId;	//GLenum lower 32-bit, Base upper 32-bit

		HashMap<u64, GLsync> fences;

		Rasterizer currRaster{ CullMode::NONE };
		BlendState currBlend{};
		DepthStencil currDepth{};

		cmd::SetClearColor clearColor{};
		i32 stencil{};
		f32 depth = 1;

		f32 minSampleShading{};

		bool enableScissor{};
		bool enableMinSampleShading{};

		//States that have to be set with commands

		GPUObjectId framebufferId{};
		GPUObjectId primitiveBufferId{};
		GPUObjectId pipelineId{};
		GPUObjectId descriptorsId{};

		//CommandList cache of active objects

		u64 frameId{}, executionId{};

	};

}