#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
#include "graphics/command/commands.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/swapchain.hpp"
#include "graphics/memory/gl_framebuffer.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"
#include "graphics/memory/gl_texture_object.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/shader/pipeline.hpp"
#include "graphics/gl_context.hpp"
#include "system/system.hpp"

namespace ignis {

	void CommandList::execute() {

		//Detect if contents are different, then revalidate command list

		for (u8 *ptr = (u8*)data.commandBuffer.data(), *end = ptr + data.next; ptr < end; ) {
			Command *c = (Command*)ptr;
			execute(c);
			ptr += c->size;
		}
	}

	void CommandList::execute(Command *c) {

		using namespace cmd;

		auto &ctx = getGraphics().getData()->getContext();

		switch (c->op) {

			case CMD_BEGIN_FRAMEBUFFER: {

				auto *fb = ((BeginFramebuffer*)c)->bindObject.get<Framebuffer>();

				if (!fb || !fb->getInfo().size.x || !fb->getInfo().size.y) {
					oic::System::log()->warn("Invalid framebuffer. Ignoring all calls that require it");
					ctx.framebuffer = nullptr;
					break;
				}

				fb->begin();
				ctx.framebuffer = fb;
				break;
			}

			case CMD_CLEAR_IMAGE: {

				const auto *ci = (ClearImage*)c;

				const auto* tex = ci->texture.get<Texture>();

				if (!tex) {
					oic::System::log()->warn("Clear image ignored; texture was invalid");
					break;
				}

				if (!(u8(tex->getInfo().usage) & u8(GPUMemoryUsage::GPU_WRITE))) {
					oic::System::log()->warn("Clear image can only be invoked on GPU writable textures");
					break;
				}

				Vec2u16 size = ci->size.x;

				if (!size.all()) {

					const Vec2i32 dif = tex->getInfo().dimensions.cast<Vec2i32>() - ci->offset.cast<Vec2i32>();

					if (!(dif > Vec2i32{}).all()) {
						oic::System::log()->warn("All values of the size should be positive");
						break;
					}

					size = dif.cast<Vec2u16>();
				}

				glxSetViewportAndScissor(ctx, size.cast<Vec2u32>(), {});

				for(u16 l = ci->minSlice; l < ci->maxSlice; ++l)
					glxClearFramebuffer(ctx, tex->getData()->framebuffer[l], 0, ctx.clearColor);

				break;
			}

			case CMD_CLEAR_BUFFER: {

				auto *cb = (ClearBuffer*)c;
				auto *buf = cb->buffer.get<GPUBuffer>();

				if (!buf) {
					oic::System::log()->warn("Clear buffer ignored; buffer was invalid");
					break;
				}

				if (cb->offset >= buf->size()) {
					oic::System::log()->warn("Clear buffer offset out of bounds");
					break;
				}

				usz size = cb->size;

				if (!size)
					size = buf->size() - cb->offset;

				if (size % 4) {
					oic::System::log()->warn("ClearBuffer can't clear individual bytes, only a scalar (4 bytes)");
					break;
				}

				glClearNamedBufferSubData(
					buf->getData()->handle,
					GL_R32UI,
					cb->offset,
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
					oic::System::log()->warn("Invalid framebuffer. Ignoring clear call");
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
						glClearNamedFramebufferfi(dat->index, GL_DEPTH_STENCIL, 0, ctx.depth, ctx.stencil);

					else {

						if(depth)
							glClearNamedFramebufferfv(dat->index, GL_DEPTH, 0, &ctx.depth);

						if (stencil)
							glClearNamedFramebufferiv(dat->index, GL_STENCIL, 0, (GLint*)&ctx.stencil);
					}
				}

				if (cf->clearFlags & ClearFramebuffer::COLOR)
					for (GLint i = 0, j = GLint(fb->size()); i < j; ++i)
						glxClearFramebuffer(ctx, dat->index, i, ctx.clearColor);

				break;
			}

			case CMD_SET_SCISSOR: {
				auto *sc = (SetScissor*)c;
				glxSetScissor(ctx, sc->size, sc->offset);
				break;
			}

			case CMD_SET_VIEWPORT: {
				auto *sv = (SetViewport*)c;
				glxSetViewport(ctx, sv->size, sv->offset);
				break;
			}

			case CMD_SET_VIEWPORT_AND_SCISSOR: {
				auto *svc = (SetViewportAndScissor*)c;
				glxSetViewportAndScissor(ctx, svc->size, svc->offset);
				break;
			}

			case CMD_END_FRAMEBUFFER:

				if(Framebuffer *fb = ctx.framebuffer)
					fb->end();

				break;

			case CMD_SET_CLEAR_COLOR: 
				ctx.clearColor = *(SetClearColor*)c;
				break;

			case CMD_SET_CLEAR_DEPTH: {

				SetClearDepth *cd = (SetClearDepth*)c;

				if (cd->dataObject != ctx.depth)
					glClearDepth(ctx.depth = cd->dataObject);

				break;
			}

			case CMD_SET_CLEAR_STENCIL: {

				SetClearStencil *cd = (SetClearStencil*)c;

				if (cd->dataObject != ctx.stencil)
					glClearStencil(ctx.stencil = cd->dataObject);

				break;
			}

			case CMD_BIND_PRIMITIVE_BUFFER: {

				auto b = ((BindPrimitiveBuffer*)c)->bindObject;
				auto *pbuffer = b.get<PrimitiveBuffer>();

				if (!pbuffer) {
					ctx.primitiveBuffer = nullptr;
					break;
				}

				if (pbuffer != ctx.primitiveBuffer) {

					ctx.primitiveBuffer = pbuffer;

					if (ctx.vaos.find(b) == ctx.vaos.end())
						ctx.vaos[b] = glxGenerateVao(pbuffer);

					GLuint vao = ctx.vaos[b];
					glBindVertexArray(vao);
				}

				break;
			}

			case CMD_BIND_PIPELINE: {

				auto *pipeline = ((BindPipeline*)c)->bindObject.get<Pipeline>();

				if (pipeline != ctx.pipeline) {

					ctx.pipeline = pipeline;

					if(pipeline)
						glxBindPipeline(ctx, pipeline);
					else
						oic::System::log()->warn("Invalid pipeline. Ignoring dispatch & draw calls");
				}

				break;
			}

			case CMD_BIND_DESCRIPTORS: {

				auto *bd = ((BindDescriptors*)c)->bindObject.get<Descriptors>();

				if (ctx.descriptors != bd) {

					ctx.descriptors = bd;

					if(bd)
						glxBindDescriptors(ctx, bd);		//TODO: Check descriptors resources
					else
						oic::System::log()->warn("Invalid descriptors. Ignoring dispatch & draw calls");
				}

				break;
			}

			case CMD_DRAW_INSTANCED: {

				if (!ctx.pipeline || !ctx.pipeline->isGraphics() || !ctx.framebuffer) {
					oic::System::log()->warn("Draw call issued without framebuffer and/or graphics pipeline");
					return;
				}

				if(
					ctx.pipeline->getInfo().attributeLayout.size() &&
					(!ctx.primitiveBuffer || !ctx.primitiveBuffer->matchLayout(ctx.pipeline->getInfo().attributeLayout))
				) {
					oic::System::log()->warn("Draw call issued with mismatching pipeline and primitive buffer layout");
					return;
				}


				if (ctx.pipeline->getInfo().pipelineLayout.size() && 
					(!ctx.descriptors || !ctx.descriptors->isShaderCompatible(
						ctx.pipeline->getInfo().pipelineLayout
					))
				) {
					oic::System::log()->warn("Pipeline layout doesn't match descriptors!");
					return;
				}

				if(ctx.framebuffer->getInfo().samples != ctx.pipeline->getInfo().msaa.samples) {
					oic::System::log()->warn("Framebuffer didn't have the same number of samples as pipeline");
					return;
				}

				auto topo = glxTopologyMode(ctx.pipeline->getInfo().topology);
				auto *di = (DrawInstanced*) c;

				if (di->isIndexed) {

					if(!ctx.primitiveBuffer) {
						oic::System::log()->warn("Primitive buffer is required for indexed drawing");
						return;
					}

					glDrawElementsInstancedBaseVertexBaseInstance(
						topo,
						di->count,
						glxGpuFormatType(ctx.primitiveBuffer->getIndexFormat()),
						(void*)(
							usz(di->start) * 
							FormatHelper::getSizeBytes(ctx.primitiveBuffer->getIndexFormat())
						),
						di->instanceCount,
						di->vertexStart,
						di->instanceStart
					);

				} else {

					if (!ctx.primitiveBuffer) {

						if (ctx.vaos.find({}) == ctx.vaos.end())
							glCreateVertexArrays(1, &ctx.vaos[{}]);

						GLuint vao = ctx.vaos[{}];
						glBindVertexArray(vao);
					}

					glDrawArraysInstancedBaseInstance(
						topo,
						di->start,
						di->count,
						di->instanceCount,
						di->instanceStart
					);
				}

				break;
			}

			case CMD_DISPATCH: {

				if (!ctx.pipeline || !ctx.pipeline->isCompute()) {
					oic::System::log()->warn("Dispatch issued without compute pipeline");
					return;
				}

				Vec3u32 threads = ((Dispatch*)c)->threadCount;
				Vec3u32 count = ctx.pipeline->getInfo().groupSize;

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

				if (!ctx.pipeline || !ctx.pipeline->isCompute()) {
					oic::System::log()->warn("No pipeline bound!");
					break;
				}

				auto *di = (DispatchIndirect*) c;
				auto *buf = di->buffer.get<GPUBuffer>();

				if (!buf) {
					oic::System::log()->warn("No indirect buffer bound!");
					break;
				}

				GLuint buffer = buf->getData()->handle;

				if(buf->size() % 16)
					oic::System::log()->fatal("Buffer should be 16-byte aligned!");

				if (ctx.bound[GL_DISPATCH_INDIRECT_BUFFER] != di->buffer) {
					glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer);
					ctx.bound[GL_DISPATCH_INDIRECT_BUFFER] = di->buffer;
				}

				glDispatchComputeIndirect(di->offset * 16);

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
					break;

			#endif

			default:
				oic::System::log()->warn("Unsupported operation");

		}

	}

}