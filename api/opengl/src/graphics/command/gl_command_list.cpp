#include "graphics/command/command_list.hpp"
#include "graphics/command/commands.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/upload_buffer.hpp"
#include "graphics/memory/swapchain.hpp"
#include "graphics/memory/gl_framebuffer.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"
#include "graphics/memory/gl_texture_object.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/shader/pipeline.hpp"
#include "graphics/gl_context.hpp"
#include "system/system.hpp"

void ::glxSetViewport(ignis::GLContext &data, const Vec2u32 &size, const Vec2i32 &offset) {

	if (data.boundApi.viewport.dim == size && data.boundApi.viewport.offset == offset)
		return;

	data.boundApi.viewport.dim = size;
	data.boundApi.viewport.offset = offset;

	glViewport(offset.x, offset.y, size.x, size.y);
}

namespace ignis {

	using namespace cmd;

	CommandList::CommandList(Graphics &g, const String &name, const Info &info):
		GPUObject(g, name, GPUObjectType::COMMAND_LIST), info(info) {}

	CommandList::~CommandList() { clear(); }

	void CommandList::execute(List<GPUObject*> &resources) {

		//Prepare commands for execution

		for (Command *c : info.commands) {

			c->prepare(getGraphics(), data);

			for (auto *res : c->getResources())
				if(res && std::find(resources.begin(), resources.end(), res) == resources.end())
					resources.push_back(res);

		}

		//Push data to GPU

		for (auto &gobj : getGraphics())
			if (gobj.first.type == GPUObjectType::UPLOAD_BUFFER)
				((UploadBuffer*)gobj.second)->flush(data, getGraphics().getData()->getContext().executionId);

		//Execute commands

		for (Command *c : info.commands)
			c->execute(getGraphics(), data);

	}

	bool glxFixSize(GLContext &data, Vec2u32 &size, const Vec2i32 &offset) {

		if (!size.x || !size.y) {

			auto asize = data.bound.framebuffer->getInfo().size.cast<Vec2i32>() - offset;

			if (!(asize >= Vec2i32()).all()) {
				oic::System::log()->error("SetViewport can't be corrected with an out of bounds offset");
				return false;
			}

			size = asize.cast<Vec2u32>();
		}

		return true;
	}

	void glxSetScissor(GLContext &data, const Vec2u32 &size, const Vec2i32 &offset) {

		if (!data.enableScissor) {
			glEnable(GL_SCISSOR_TEST);
			data.enableScissor = true;
		}

		if (data.boundApi.viewport.dim == size && data.boundApi.viewport.offset == offset)
			return;

		data.boundApi.scissor.dim = size;
		data.boundApi.scissor.offset = offset;

		glScissor(offset.x, offset.y, size.x, size.y);
	}

	bool glxBindDescriptors(GLContext &ctx) {

		auto *descriptors = ctx.bound.descriptors;

		if (
			ctx.bound.pipeline->getInfo().pipelineLayout &&
			ctx.bound.pipeline->getInfo().pipelineLayout->getInfo().size() && 
			(!descriptors || !descriptors->isShaderCompatible(ctx.bound.pipeline->getInfo().pipelineLayout))
		) {
			oic::System::log()->error("Pipeline layout doesn't match descriptors!");
			return false;
		}

		::glxBindDescriptors(ctx, ctx.boundApi.descriptors = descriptors);

		return true;
	}

	bool glxPrepareGraphicsPipeline(GLContext &ctx) {

		auto *pipeline = ctx.bound.pipeline;

		//Validate & bind pipeline

		if (!pipeline || !pipeline->isGraphics()) {
			oic::System::log()->error("No graphics pipeline bound!");
			return false;
		}

		if(ctx.boundApi.pipeline != pipeline)
			glxBindPipeline(ctx, ctx.boundApi.pipeline = pipeline);

		//Validate & bind descriptors

		if (!glxBindDescriptors(ctx))
			return false;

		//Bind primitive buffer

		auto *primitiveBuffer = ctx.bound.primitiveBuffer;

		if (ctx.boundApi.primitiveBuffer != primitiveBuffer) {

			if(
				pipeline->getInfo().attributeLayout.size() &&
				(!primitiveBuffer || !primitiveBuffer->matchLayout(pipeline->getInfo().attributeLayout))
			) {
				oic::System::log()->error("Draw call issued with mismatching pipeline and primitive buffer layout");
				return false;
			}

			ctx.boundApi.primitiveBuffer = primitiveBuffer;

			auto id = getGPUObjectId(primitiveBuffer);

			if (ctx.vaos.find(id) == ctx.vaos.end())
				if (id.null())
					glCreateVertexArrays(1, &ctx.vaos[id]);
				else
					if (GLuint vao = glxGenerateVao(primitiveBuffer))
						ctx.vaos[id] = vao;
					else
						return false;

			glBindVertexArray(ctx.vaos[id]);
		}

		//Bind & validate framebuffer

		auto *framebuffer = ctx.bound.framebuffer;

		if(!framebuffer) {
			oic::System::log()->error("Framebuffer is required for draw calls");
			return false;
		}

		if(framebuffer->getInfo().samples != pipeline->getInfo().msaa.samples) {
			oic::System::log()->error("Framebuffer didn't have the same number of samples as pipeline");
			return false;
		}

		if (ctx.boundApi.framebuffer != framebuffer) {
			glxBeginRenderPass(ctx, framebuffer->getId(), framebuffer->getData()->handle);
			ctx.boundApi.framebuffer = framebuffer;
		}

		//Bind viewport & scissor

		Vec2u32 viewportSize = ctx.bound.viewport.dim, scissorSize = ctx.bound.scissor.dim;
		Vec2i32 viewportOffset = ctx.bound.viewport.offset, scissorOffset = ctx.bound.scissor.offset;

		if (!glxFixSize(ctx, viewportSize, viewportOffset)) return false;
		if (!glxFixSize(ctx, scissorSize, scissorOffset)) return false;

		if(viewportSize != scissorSize || viewportOffset != scissorOffset)
			glxSetScissor(ctx, scissorSize, scissorOffset);
		else if (ctx.enableScissor) {
			glDisable(GL_SCISSOR_TEST);
			ctx.enableScissor = false;
		}

		glxSetViewport(ctx, viewportSize, viewportOffset);

		return true;
	}

	bool glxPrepareComputePipeline(GLContext &ctx) {

		auto *pipeline = ctx.bound.pipeline;

		if (!pipeline || !pipeline->isCompute()) {
			oic::System::log()->error("No compute pipeline bound!");
			return false;
		}

		if (pipeline != ctx.boundApi.pipeline)
			glxBindPipeline(ctx, ctx.boundApi.pipeline = pipeline);

		if (!glxBindDescriptors(ctx))
			return false;

		return true;
	}

	#define context g.getData()->getContext()

	//Binding and setting things in the context

	void BindPipeline::execute(Graphics &g, CommandList::Data*) const {

		context.bound.pipeline = pipeline;

		if(pipeline.null())
			oic::System::log()->error("Invalid pipeline. Ignoring dispatch & draw calls");
	}

	void BindDescriptors::execute(Graphics &g, CommandList::Data*) const { context.bound.descriptors = descriptors; }
	void BindPrimitiveBuffer::execute(Graphics &g, CommandList::Data*) const { context.bound.primitiveBuffer = primitiveBuffer; }

	void SetStencil::execute(Graphics &g, CommandList::Data*) const { context.stencil = stencil; }
	void SetClearDepth::execute(Graphics &g, CommandList::Data*) const { context.depth = depth; }
	void SetClearColor::execute(Graphics &g, CommandList::Data*) const { context.clearColor = *this;}
	void SetScissor::execute(Graphics &g, CommandList::Data*) const { context.bound.scissor = *this; }
	void SetViewport::execute(Graphics &g, CommandList::Data*) const { context.bound.viewport = *this; }

	void SetViewportAndScissor::execute(Graphics &g, CommandList::Data*) const { 
		context.bound.viewport = (const SetViewport&)*this; 
		context.bound.scissor = (const SetScissor&)*this; 
	}

	//Begin / end

	void BeginFramebuffer::execute(Graphics &g, CommandList::Data*) const { 

		Framebuffer *fb = framebuffer;

		if (!fb || !fb->getInfo().size.x || !fb->getInfo().size.y) {
			oic::System::log()->error("Invalid framebuffer. Ignoring all calls that require it");
			context.bound.framebuffer = nullptr;
			return;
		}

		context.bound.framebuffer = fb; 
	}

	void EndFramebuffer::execute(Graphics &g, CommandList::Data*) const { context.bound.framebuffer = nullptr; }

	//Draw and dispatches

	void DrawInstanced::execute(Graphics &g, CommandList::Data*) const {
	
		auto &ctx = context;

		if (!glxPrepareGraphicsPipeline(ctx)) {
			oic::System::log()->error("Draw instanced call ignored because the graphics pipeline wasn't valid");
			return;
		}

		auto topo = glxTopologyMode(ctx.bound.pipeline->getInfo().topology);

		if (isIndexed) {

			if(!ctx.bound.primitiveBuffer) {
				oic::System::log()->error("Primitive buffer is required for indexed drawing");
				return;
			}

			glDrawElementsInstancedBaseVertexBaseInstance(
				topo,
				count,
				glxGpuFormatType(ctx.bound.primitiveBuffer->getIndexFormat()),
				(void*)(
					usz(start) * 
					FormatHelper::getSizeBytes(ctx.bound.primitiveBuffer->getIndexFormat())
				),
				instanceCount,
				vertexStart,
				instanceStart
			);
		}

		else glDrawArraysInstancedBaseInstance(
			topo,
			start,
			count,
			instanceCount,
			instanceStart
		);
	}

	void Dispatch::execute(Graphics &g, CommandList::Data*) const {

		auto &ctx = context;

		if (!glxPrepareComputePipeline(ctx)) {
			oic::System::log()->error("Dispatch issued without compute pipeline");
			return;
		}

		const Vec3u32 &threads = threadCount;
		Vec3u32 count = ctx.bound.pipeline->getInfo().groupSize;

		Vec3u32 groups = (threads.cast<Vec3f32>() / count.cast<Vec3f32>()).ceil().cast<Vec3u32>();

		#ifndef NDEBUG
			if ((threads % count).any())
				oic::System::log()->performance(
					"Thread count was incompatible with compute shader "
					"this is fixed by the runtime, but could provide out of "
					"bounds texture writes or reads"
				);
		#endif

		glDispatchCompute(groups.x, groups.y, groups.z);
	}

	void DispatchIndirect::execute(Graphics &g, CommandList::Data*) const {

		auto &ctx = context;

		if (!glxPrepareComputePipeline(ctx)) {
			oic::System::log()->error("Dispatch indirect issued without compute pipeline");
			return;
		}

		GPUBuffer *buf = buffer;

		if (!buf) {
			oic::System::log()->error("No indirect buffer bound!");
			return;
		}

		GLuint handle = buf->getData()->handle;

		if(buf->size() % 16)
			oic::System::log()->fatal("Buffer should be 16-byte aligned!");

		if (ctx.boundObjects[GL_DISPATCH_INDIRECT_BUFFER] != buf->getId()) {
			glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, handle);
			ctx.boundObjects[GL_DISPATCH_INDIRECT_BUFFER] = buf->getId();
		}

		glDispatchComputeIndirect(GLintptr(offset));
	}

	//Clearing

	void ClearFramebuffer::execute(Graphics &g, CommandList::Data*) const {
	
		auto &ctx = context;
		Framebuffer *fb = ctx.bound.framebuffer;

		if (!fb || !fb->getInfo().size.x || !fb->getInfo().size.y) {
			oic::System::log()->error("Invalid framebuffer. Ignoring clear call");
			return;
		}

		auto *dat = fb->getData();

		if (fb->getDepth()) {

			bool depth = clearFlags & ClearFramebuffer::DEPTH;

			bool stencil = 
				clearFlags & ClearFramebuffer::STENCIL && 
				FormatHelper::hasStencil(fb->getInfo().depthFormat);

			if(ctx.currDepth.enableDepthWrite != true)
				glDepthMask(ctx.currDepth.enableDepthWrite = true);

			//TODO: Test stencil buffers, since they might need stencil write to be turned on

			if(depth && stencil)
				glClearNamedFramebufferfi(dat->handle, GL_DEPTH_STENCIL, 0, ctx.depth, ctx.stencil);

			else {

				if(depth)
					glClearNamedFramebufferfv(dat->handle, GL_DEPTH, 0, &ctx.depth);

				if (stencil)
					glClearNamedFramebufferiv(dat->handle, GL_STENCIL, 0, &ctx.stencil);
			}
		}

		if (clearFlags & ClearFramebuffer::COLOR)
			for (GLint i = 0, j = GLint(fb->size()); i < j; ++i)
				glxClearFramebuffer(ctx, dat->handle, i, ctx.clearColor);

	}

	void ClearImage::execute(Graphics &g, CommandList::Data*) const {

		if (!texture) {
			oic::System::log()->error("Clear image ignored; texture was invalid");
			return;
		}

		if (!HasFlags(texture->getInfo().usage, GPUMemoryUsage::GPU_WRITE)) {
			oic::System::log()->error("Clear image can only be invoked on GPU writable textures");
			return;
		}

		Vec2u16 siz = size;

		if (!size.all()) {

			const Vec2i32 dif = texture->getInfo().dimensions.cast<Vec2i32>() - offset.cast<Vec2i32>();

			if (!(dif > Vec2i32{}).all()) {
				oic::System::log()->error("All values of the size should be positive");
				return;
			}

			siz = dif.cast<Vec2u16>();
		} 

		auto &ctx = context;

		glxSetViewport(ctx, siz.cast<Vec2u32>(), offset.cast<Vec2i32>());

		if (ctx.enableScissor) {
			glDisable(GL_SCISSOR_TEST);
			ctx.enableScissor = false;
		}

		for(u16 l = slice; l < slice + slices; ++l)
			for(u16 m = mipLevels; m < mipLevel + mipLevels; ++m)
				glxClearFramebuffer(
					ctx, texture->getData()->framebuffer[size_t(l) * texture->getInfo().mips + m], 0, ctx.clearColor
				);
	}

	void ClearBuffer::execute(Graphics&, CommandList::Data*) const {
	
		if (!buffer) {
			oic::System::log()->error("Clear buffer ignored; buffer was invalid");
			return;
		}

		if (!HasFlags(buffer->getInfo().usage, GPUMemoryUsage::GPU_WRITE)) {
			oic::System::log()->error("Clear buffer can only be invoked on GPU writable buffers");
			return;
		}

		u64 size = elements;

		if (offset + size >= buffer->size()) {
			oic::System::log()->error("Clear buffer out of bounds");
			return;
		}

		if (!size)
			size = buffer->size() - offset;

		if (!size) return;

		if (size & 3 || offset & 3) {
			oic::System::log()->error("ClearBuffer can't clear individual bytes, only a scalar (4 bytes)");
			return;
		}

		glClearNamedBufferSubData(
			buffer->getData()->handle,
			GL_R32UI,
			offset, size,
			GL_RED_INTEGER, GL_UNSIGNED_INT,
			nullptr
		);
	}

	//Debugging

	#ifndef NDEBUG

		void DebugStartRegion::execute(Graphics&, CommandList::Data*) const {
			glPushDebugGroup(
				GL_DEBUG_SOURCE_APPLICATION,
				0,
				GLsizei(name.size()),
				name.c_str()
			);
		}

		void DebugInsertMarker::execute(Graphics&, CommandList::Data*) const {
			glDebugMessageInsert(
				GL_DEBUG_SOURCE_APPLICATION,
				GL_DEBUG_TYPE_MARKER,
				0,
				GL_DEBUG_SEVERITY_NOTIFICATION,
				GLsizei(name.size()),
				name.c_str()
			);
		}

		void DebugEndRegion::execute(Graphics&, CommandList::Data*) const {
			glPopDebugGroup();
		}

	#else

		void DebugStartRegion::execute(Graphics&, CommandList::Data*) const { }
		void DebugInsertMarker::execute(Graphics&, CommandList::Data*) const { }
		void DebugEndRegion::execute(Graphics&, CommandList::Data*) const { }

	#endif

}