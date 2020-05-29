#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
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

	CommandList::CommandList(Graphics &g, const String &name, const Info &info):
		GPUObject(g, name, GPUObjectType::COMMAND_LIST), info(info) {}

	CommandList::~CommandList() {}

	void CommandList::execute() {

		//Detect if contents are different, then revalidate command list

		for (u8 *ptr = (u8*)info.commandBuffer.data(), *end = ptr + info.next; ptr < end; ) {
			Command *c = (Command*)ptr;
			execute(c);
			ptr += c->commandSize;
		}
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

		if(ctx.boundApi.descriptors != descriptors)
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

	void CommandList::execute(Command *c) {

		using namespace cmd;

		auto &ctx = getGraphics().getData()->getContext();

		switch (c->op) {

			case CMD_BEGIN_FRAMEBUFFER: {

				auto *fb = ((BeginFramebuffer*)c)->bindObject.get<Framebuffer>();

				if (!fb || !fb->getInfo().size.x || !fb->getInfo().size.y) {
					oic::System::log()->error("Invalid framebuffer. Ignoring all calls that require it");
					ctx.bound.framebuffer = nullptr;
					break;
				}

				ctx.bound.framebuffer = fb;
				break;
			}

			case CMD_END_FRAMEBUFFER:
				ctx.bound.framebuffer = nullptr;
				break;

			case CMD_SET_CLEAR_COLOR: 
				ctx.clearColor = *(SetClearColor*)c;
				break;

			case CMD_SET_CLEAR_DEPTH:
				ctx.depth = ((SetClearDepth*)c)->dataObject;
				break;

			case CMD_SET_CLEAR_STENCIL:
				ctx.stencil = ((SetClearStencil*)c)->dataObject;
				break;

			case CMD_CLEAR_IMAGE: {

				const auto *ci = (ClearImage*)c;

				const auto* tex = ci->texture.get<Texture>();

				if (!tex) {
					oic::System::log()->error("Clear image ignored; texture was invalid");
					break;
				}

				if (!HasFlags(tex->getInfo().usage, GPUMemoryUsage::GPU_WRITE)) {
					oic::System::log()->error("Clear image can only be invoked on GPU writable textures");
					break;
				}

				Vec2u16 size = ci->size;

				if (!size.all()) {

					const Vec2i32 dif = tex->getInfo().dimensions.cast<Vec2i32>() - ci->offset.cast<Vec2i32>();

					if (!(dif > Vec2i32{}).all()) {
						oic::System::log()->error("All values of the size should be positive");
						break;
					}

					size = dif.cast<Vec2u16>();
				}

				glxSetViewport(ctx, size.cast<Vec2u32>(), ci->offset.cast<Vec2i32>());

				if (ctx.enableScissor) {
					glDisable(GL_SCISSOR_TEST);
					ctx.enableScissor = false;
				}

				for(u16 l = ci->slice; l < ci->slice + ci->slices; ++l)
					for(u16 m = ci->mipLevels; m < ci->mipLevel + ci->mipLevels; ++m)
						glxClearFramebuffer(
							ctx, tex->getData()->framebuffer[size_t(l) * tex->getInfo().mips + m], 0, ctx.clearColor
						);

				break;
			}

			case CMD_CLEAR_BUFFER: {

				auto *cb = (ClearBuffer*)c;
				auto *buf = cb->buffer.get<GPUBuffer>();

				if (!buf) {
					oic::System::log()->error("Clear buffer ignored; buffer was invalid");
					break;
				}

				if (!HasFlags(buf->getInfo().usage, GPUMemoryUsage::GPU_WRITE)) {
					oic::System::log()->error("Clear buffer can only be invoked on GPU writable buffers");
					break;
				}

				u64 size = cb->elements;
				const u64 offset = cb->offset;

				if (offset + size >= buf->size()) {
					oic::System::log()->error("Clear buffer out of bounds");
					break;
				}

				if (!size)
					size = buf->size() - offset;

				if (!size) break;

				if (size & 3 || offset & 3) {
					oic::System::log()->error("ClearBuffer can't clear individual bytes, only a scalar (4 bytes)");
					break;
				}

				glClearNamedBufferSubData(
					buf->getData()->handle,
					GL_R32UI,
					offset,
					size,
					GL_RED_INTEGER,
					GL_UNSIGNED_INT,
					nullptr
				);

				break;
			}
			
			case CMD_CLEAR_FRAMEBUFFER: {

				auto *cf = (ClearFramebuffer*)c;
				Framebuffer *fb = cf->target.get<Framebuffer>();

				if (!fb || !fb->getInfo().size.x || !fb->getInfo().size.y) {
					oic::System::log()->error("Invalid framebuffer. Ignoring clear call");
					break;
				}

				auto *dat = fb->getData();

				if (fb->getDepth()) {

					bool depth = cf->clearFlags & ClearFramebuffer::DEPTH;

					bool stencil = 
						cf->clearFlags & ClearFramebuffer::STENCIL && 
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

				if (cf->clearFlags & ClearFramebuffer::COLOR)
					for (GLint i = 0, j = GLint(fb->size()); i < j; ++i)
						glxClearFramebuffer(ctx, dat->handle, i, ctx.clearColor);

				break;
			}

			case CMD_SET_SCISSOR:
				ctx.bound.scissor = *(SetScissor*)c;
				break;

			case CMD_SET_VIEWPORT:
				ctx.bound.viewport = *(SetViewport*)c;
				break; 

			case CMD_SET_VIEWPORT_AND_SCISSOR:
				ctx.bound.scissor = *(SetScissor*)c;
				ctx.bound.viewport = *(SetViewport*)c;
				break; 

			case CMD_BIND_PRIMITIVE_BUFFER:
				ctx.bound.primitiveBuffer = (((BindPrimitiveBuffer*)c)->bindObject).get<PrimitiveBuffer>();
				break;

			case CMD_BIND_PIPELINE:

				ctx.bound.pipeline = ((BindPipeline*)c)->bindObject.get<Pipeline>();

				if(!ctx.bound.pipeline)
					oic::System::log()->error("Invalid pipeline. Ignoring dispatch & draw calls");

				break;

			case CMD_BIND_DESCRIPTORS:
				ctx.bound.descriptors = ((BindDescriptors*)c)->bindObject.get<Descriptors>();
				break;

			case CMD_DRAW_INSTANCED: {

				if (!glxPrepareGraphicsPipeline(ctx)) {
					oic::System::log()->error("Draw instanced call ignored because the graphics pipeline wasn't valid");
					break;
				}

				auto topo = glxTopologyMode(ctx.bound.pipeline->getInfo().topology);
				auto *di = (DrawInstanced*) c;

				if (di->isIndexed) {

					if(!ctx.bound.primitiveBuffer) {
						oic::System::log()->error("Primitive buffer is required for indexed drawing");
						break;
					}

					glDrawElementsInstancedBaseVertexBaseInstance(
						topo,
						di->count,
						glxGpuFormatType(ctx.bound.primitiveBuffer->getIndexFormat()),
						(void*)(
							usz(di->start) * 
							FormatHelper::getSizeBytes(ctx.bound.primitiveBuffer->getIndexFormat())
						),
						di->instanceCount,
						di->vertexStart,
						di->instanceStart
					);
				}

				else glDrawArraysInstancedBaseInstance(
					topo,
					di->start,
					di->count,
					di->instanceCount,
					di->instanceStart
				);

				break;
			}

			case CMD_DISPATCH: {

				if (!glxPrepareComputePipeline(ctx)) {
					oic::System::log()->error("Dispatch issued without compute pipeline");
					break;
				}

				Vec3u32 threads = ((Dispatch*)c)->threadCount;
				Vec3u32 count = ctx.bound.pipeline->getInfo().groupSize;

				Vec3u32 groups = (threads.cast<Vec3f32>() / count.cast<Vec3f32>()).ceil().cast<Vec3u32>();

				if ((threads % count).any())
					oic::System::log()->performance(
						"Thread count was incompatible with compute shader "
						"this is fixed by the runtime, but could provide out of "
						"bounds texture writes or reads"
					);

				glDispatchCompute(groups.x, groups.y, groups.z);
				break;
			}

			case CMD_DISPATCH_INDIRECT: {

				if (!glxPrepareComputePipeline(ctx)) {
					oic::System::log()->error("Dispatch indirect issued without compute pipeline");
					break;
				}

				auto *di = (DispatchIndirect*) c;
				auto *buf = di->buffer.get<GPUBuffer>();

				if (!buf) {
					oic::System::log()->error("No indirect buffer bound!");
					break;
				}

				GLuint buffer = buf->getData()->handle;

				if(buf->size() % 16)
					oic::System::log()->fatal("Buffer should be 16-byte aligned!");

				if (ctx.boundObjects[GL_DISPATCH_INDIRECT_BUFFER] != di->buffer) {
					glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer);
					ctx.boundObjects[GL_DISPATCH_INDIRECT_BUFFER] = di->buffer;
				}

				glDispatchComputeIndirect(GLintptr(di->offset) * 16);

				break;
			}

			#ifndef NDEBUG

				case CMD_DEBUG_START_REGION: {

					auto *sr = (DebugStartRegion*)c;

					glPushDebugGroup(
						GL_DEBUG_SOURCE_APPLICATION,
						0,
						GLsizei(sr->size()),
						sr->string
					);

					break;
				}

				case CMD_DEBUG_END_REGION:
					glPopDebugGroup();
					break;

				case CMD_DEBUG_INSERT_MARKER: {

					auto *im = (DebugInsertMarker*)c;

					glDebugMessageInsert(
						GL_DEBUG_SOURCE_APPLICATION,
						GL_DEBUG_TYPE_MARKER,
						0,
						GL_DEBUG_SEVERITY_NOTIFICATION,
						GLsizei(im->size()),
						im->string
					);

					break;
				}

			#else

				case CMD_DEBUG_START_REGION:
				case CMD_DEBUG_END_REGION:
				case CMD_DEBUG_INSERT_MARKER:
					oic::System::log()->performance("Using debug GPU instructions in release mode costs performance");
					break;

			#endif

			case CMD_FLUSH_BUFFER: {

				auto *fb = (FlushBuffer*)c;

				auto *uploadBuffer = fb->uploadBuffer.get<UploadBuffer>();

				if (fb->type == FlushBuffer::PRIMITIVE_BUFFER) {

					PrimitiveBuffer *buf = fb->buffer.get<PrimitiveBuffer>();

					if (!buf) {
						oic::System::log()->error("Flush buffer called without buffer");
						break;
					}

					auto &buffers = buf->getVertexBuffers();

					for(auto &b : buffers)
						b.buffer->flush(data, uploadBuffer, b.bufferOffset, b.size());

					auto &index = buf->getIndexBuffer();

					if (index.buffer)
						index.buffer->flush(data, uploadBuffer, index.bufferOffset, index.size());

					break;
				}

				else if (fb->type != FlushBuffer::REGULAR) {

					PrimitiveBuffer *buf = fb->buffer.get<PrimitiveBuffer>();

					if (!buf) {
						oic::System::log()->error("Flush primitive buffer called without buffer");
						break;
					}

					if (fb->type == FlushBuffer::INDEX) {

						if (!buf->hasIndices()) {
							oic::System::log()->error("Flush index buffer called without indices");
							break;
						}

						auto &ibo = buf->getIndexBuffer();

						if (ibo.stride() * (fb->offset + fb->elements) > ibo.size()) {
							oic::System::log()->error("Flush index buffer out of bounds");
							break;
						}

						auto bufferStart = ibo.stride() * fb->offset;

						ibo.buffer->flush(data, uploadBuffer, ibo.bufferOffset + bufferStart, fb->elements ? ibo.stride() * fb->elements : ibo.size() - bufferStart);

						break;
					}

					auto &buffers = buf->getVertexBuffers();

					for (auto &b : buffers) {

						if (b.stride() * (fb->offset + fb->elements) > b.size()) {
							oic::System::log()->error("Flush vertex buffer out of bounds");
							break;
						}

						auto bufferStart = b.stride() * fb->offset;

						b.buffer->flush(data, uploadBuffer, b.bufferOffset + bufferStart, fb->elements ? b.stride() * fb->elements : b.size() - bufferStart);
					}

					break;
				}

				GPUBuffer *buf = dynamic_cast<GPUBuffer*>(fb->buffer.get<GPUObject>());

				if (!buf) {
					oic::System::log()->error("Flush buffer called without buffer");
					break;
				}

				if (fb->offset + fb->elements > buf->size()) {
					oic::System::log()->error("Flush buffer out of bounds");
					break;
				}

				buf->flush(data, uploadBuffer, fb->offset, fb->elements ? fb->elements : buf->size() - fb->offset);
				break;
			}

			case CMD_FLUSH_IMAGE: {

				auto *fi = (FlushImage*)c;

				Texture *tex = fi->image.get<Texture>();

				if (!tex) {
					oic::System::log()->error("Flush image called without texture");
					break;
				}

				tex->flush(data, fi->uploadBuffer.get<UploadBuffer>(), fi->mipStart, fi->mipCount);
				break;
			}

			case CMD_TRACE_RAYS_FT_2:
			case CMD_BUILD_ACCELERATION_STRUCTURE_FT_2:
			case CMD_COPY_ACCELERATION_STRUCTURE_FT_2:
			case CMD_WRITE_ACCELERATION_STRUCTURE_PROPERTIES_FT_2:
				oic::System::log()->error("Raytracing isn't supported in OpenGL, please compile using an API that supports it");
				break;

			default:
				oic::System::log()->error("Unsupported operation");

		}

	}

}