#pragma once
#include "graphics/command/command_list.hpp"
#include "graphics/shader/pipeline.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/memory/framebuffer.hpp"
#include "graphics/memory/texture.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/upload_buffer.hpp"
#include "types/vec.hpp"
#include <cstring>

//General GPU commands
//These have to be implemented for every CommandList implementation

namespace ignis {

	class Pipeline;
	class Framebuffer;
	class Descriptors;
	class PrimitiveBuffer;
	class Query;
	class GPUBuffer;
	class UploadBuffer;
	class Texture;

	namespace cmd {

		//Basic bind commands

		class BindPipeline : public Command {

			PipelineRef pipeline;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			BindPipeline(Pipeline *pipeline): pipeline(pipeline) {}
		};

		class BindDescriptors : public Command {

			DescriptorsRef descriptors;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			BindDescriptors(Descriptors *descriptors): descriptors(descriptors) {}
		};

		class BindPrimitiveBuffer : public Command {

			PrimitiveBufferRef primitiveBuffer;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			BindPrimitiveBuffer(PrimitiveBuffer *primitiveBuffer): primitiveBuffer(primitiveBuffer) {}
		};

		//Basic begin/end commands

		class BeginFramebuffer : public Command {

			FramebufferRef framebuffer;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			BeginFramebuffer(Framebuffer *framebuffer): framebuffer(framebuffer) {}
		};

		class EndFramebuffer : public Command {
			apimpl void execute(Graphics&, CommandList::Data*) const final override;
		};

		//TODO: Begin/end query

		//Draw/dispatch commands

		//Indexed or non-indexed; instanced draw call
		//DrawInstanced(...) for non-indexed
		//DrawInstanced::indexed() for indexed
		class DrawInstanced : public Command {

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

			u32 start, count, instanceCount, instanceStart, vertexStart;
			bool isIndexed;

		public:

			DrawInstanced(
				u32 count, u32 instanceCount = 1, u32 start = 0, u32 instanceStart = 0
			): 
				start(start), count(count), instanceCount(instanceCount), 
				instanceStart(instanceStart), vertexStart(), isIndexed(false) {}

			static inline DrawInstanced indexed(u32 count, u32 instanceCount = 1, u32 start = 0, u32 instanceStart = 0, u32 vertexStart = 0) {
				DrawInstanced di(count, instanceCount, start, instanceStart);
				di.vertexStart = vertexStart;
				di.isIndexed = true;
				return di;
			}
		};

		//Dispatch calls

		class Dispatch : public Command {

			Vec3u32 threadCount;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			Dispatch(u32 threads) :
				threadCount { threads, 1, 1 } {}

			Dispatch(const Vec2u32 &threads) :
				threadCount { threads.x, threads.y, 1 } {}

			Dispatch(const Vec3u32 &threads) :
				threadCount(threads) {}

		};

		class DispatchIndirect : public Command {

			GPUBufferRef buffer;
			u64 offset;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			DispatchIndirect(GPUBuffer *buffer, u32 offset = 0) :
				buffer(buffer),
				offset(offset << 4_u64) {}

		};

		//Setting values

		class SetStencil : public Command {

			u8 stencil;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			SetStencil(u8 stencil): stencil(stencil) {}
		};

		class SetClearDepth : public Command {

			f32 depth;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			SetClearDepth(f32 depth): depth(depth) {}
		};

		struct SetClearColor : public Command {

			enum class Type : u8 {
				FLOAT, UNSIGNED_INT, SIGNED_INT
			};

			union {
				Vec4f32 rgbaf;
				Vec4u32 rgbau;
				Vec4i32 rgbai;
			};

			Type type;

			SetClearColor(): SetClearColor(Vec4f32()) {}
			SetClearColor(const Vec4f32 &rgba): rgbaf(rgba), type(Type::FLOAT) {}
			SetClearColor(const Vec4u32 &rgba): rgbau(rgba), type(Type::UNSIGNED_INT) {}
			SetClearColor(const Vec4i32 &rgba): rgbai(rgba), type(Type::SIGNED_INT) {}

		private:

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		};

		struct SetViewRegion : public Command {

			Vec2i32 offset;
			Vec2u32 dim;

		protected:

			SetViewRegion(const Vec2u32 &dim, const Vec2i32 &offset): offset(offset), dim(dim) {}
		};

		class SetScissor : public SetViewRegion {

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			SetScissor(const Vec2u32 &dim = {}, const Vec2i32 &offset = {}) : SetViewRegion(dim, offset) {}
		};

		class SetViewport : public SetViewRegion {

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			SetViewport(const Vec2u32 &dim = {}, const Vec2i32 &offset = {}) : SetViewRegion(dim, offset) {}
		};

		class SetViewportAndScissor : public SetViewRegion {

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			SetViewportAndScissor(const Vec2u32 &dim = {}, const Vec2i32 &offset = {}) : SetViewRegion(dim, offset) {}
		};

		//Copy commands

		//Clears framebuffer to the clear color/depth/stencil (depending on the flags you set)
		struct ClearFramebuffer : Command {

			enum ClearFlags : u8 {
				COLOR = 1,
				DEPTH = 2,
				STENCIL = 4,
				DEPTH_STENCIL = 6,
				ALL = 7
			};

		private:

			ClearFlags clearFlags;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			ClearFramebuffer(ClearFlags clearFlags = ALL): clearFlags(clearFlags) {}
		};

		//Clears image to the clear color (like framebuffer)
		class ClearImage : Command {

			TextureRef texture;
			Vec2i16 offset;
			Vec2u16 size;
			u16 mipLevel, mipLevels;
			u16 slice, slices;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			ClearImage(
				Texture *texture,
				u16 mipLevel = {}, u16 mipLevels = {}, u16 slice = {}, u16 slices = {},
				const Vec2u16 &size = {}, const Vec2i16 &offset = {}
			) :
				texture(texture), mipLevel(mipLevel), slice(slice),
				offset(offset), size(size), slices(slices ? slices : 1), mipLevels(mipLevels ? mipLevels : 1) {}
		};

		//Clears buffer to zero
		class ClearBuffer : Command {

			GPUBufferRef buffer;
			u64 offset, elements;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			ClearBuffer(GPUBuffer *buffer, u64 offset = 0, u64 elements = 0) :
				buffer(buffer), offset(offset), elements(elements) {}
		};

		//Transfer calls

		class FlushBuffer : Command {

			GPUBufferRef gbuffer;
			PrimitiveBufferRef pbuffer;

			UploadBufferRef uploadBuffer;

			List<Pair<u64, u64>> flushedRanges;

			apimpl void prepare(Graphics&, CommandList::Data*) final override;
			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			FlushBuffer(GPUBuffer *buffer, UploadBuffer *uploadBuffer):
				gbuffer(buffer), uploadBuffer(uploadBuffer) {}

			FlushBuffer(PrimitiveBuffer *buffer, UploadBuffer *uploadBuffer):
				pbuffer(buffer), uploadBuffer(uploadBuffer) {}

		};

		class FlushImage : Command {

			TextureRef image;
			UploadBufferRef uploadBuffer;

			Pair<u64, u64> flushedRange;

			apimpl void prepare(Graphics&, CommandList::Data*) final override;
			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			//Command for flushing CPU data to the GPU
			//ringBuffer must be non zero if the resource isn't allocated in shared memory (is queryable on Texture)
			//GPU memory must be accessible
			//
			FlushImage(Texture *tex, UploadBuffer *uploadBuffer):
				image(tex), uploadBuffer(uploadBuffer) {}

		};
		
		//Debug calls

		class DebugMarkerCommand : public DebugCommand {

		protected:

			String name;
			Vec4f32 colorExt;	//Sometimes color coding your markers is allowed by the API

		public:

			DebugMarkerCommand(const String &str, const Vec4f32 &colorExt) :
				name(str), colorExt(colorExt) {}
		};

		class DebugStartRegion : public DebugMarkerCommand {

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			DebugStartRegion(const String &str, const Vec4f32 &colorExt = Vec4f32(1, 0, 0, 1)) :
				DebugMarkerCommand(str, colorExt) {}
		};

		class DebugInsertMarker : public DebugMarkerCommand {

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			DebugInsertMarker(const String &str, const Vec4f32 &colorExt = Vec4f32(1, 0, 0, 1)) :
				DebugMarkerCommand(str, colorExt) {}
		};

		class DebugEndRegion : public DebugCommand {
			apimpl void execute(Graphics&, CommandList::Data*) const final override;
		};

		//Extensions

		/*

		class TODO: RTTraceFt : public CommandFt {

			ShaderBindingTable shaderBindingTable;

			List<usz> raygenEntries, missEntries, hitShaders, callableShaders;

			Vec3u32 dimensions;

			apimpl void execute(Graphics&, CommandList::Data*) const final override;

		public:

			RTTraceFt(const Vec3u32 &dim): CommandFt(Feature::RAY_TRACING), dimensions(dim) {}

		};

		TODO: Build, copy, write

		*/

	}

}