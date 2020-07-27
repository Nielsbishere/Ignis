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

		struct Execution {

			GLsync sync{};
			List<GPUObject*> objects;

			void *callbackObject{};
			void (*functionPtr)(void*, UploadBuffer*, const Pair<u64, u64>&, TextureObject*, const Vec3u16&, const Vec3u16&, u16, u8, bool){};

			TextureObject *gpuTexture{};
			UploadBuffer *cpuOutput{};
			Pair<u64, u64> allocation{};
			Vec3u16 offset{};
			Vec3u16 size{};
			u16 layer{};
			u8 mip{};
			bool isStencil{};

			inline void call() const {
				if (auto func = functionPtr)
					func(callbackObject, cpuOutput, allocation, gpuTexture, offset, size, layer, mip, isStencil);
			}

		};

		HashMap<u64, Execution> fences;

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