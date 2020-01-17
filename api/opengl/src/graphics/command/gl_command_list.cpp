#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
#include "graphics/command/commands.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/swapchain.hpp"
#include "graphics/memory/gl_framebuffer.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/shader/pipeline.hpp"
#include "graphics/gl_context.hpp"
#include "system/system.hpp"

namespace ignis {

	void CommandList::execute(Command *c) {

		using namespace cmd;

		auto &ctx = getGraphics().getData()->getContext();

		//TODO: Instead of doing validation here; 
		//do it whenever a command enters the command buffer!
		//Also make the arguments invalid so they can pass through
		//Also things like begin/end without a framebuffer set

		switch (c->op) {

			case CMD_BEGIN_FRAMEBUFFER: {

				auto *bs = (BeginFramebuffer*)c;

				auto size = bs->target->getInfo().size;

				if(!size[0] || !size[1])
					oic::System::log()->fatal("Please specify an initialized framebuffer");

				bs->target->begin();
				ctx.currentFramebuffer = bs->target;
				break;
			}

			case CMD_CLEAR_FRAMEBUFFER: {

				auto *cf = (ClearFramebuffer*)c;
				auto *targ = cf->target;
				auto *dat = targ->getData();

				if (DepthTexture *dt = targ->getDepth()) {

					bool depth = cf->clearFlags & ClearFramebuffer::DEPTH;

					bool stencil = 
						cf->clearFlags & ClearFramebuffer::STENCIL && 
						FormatHelper::hasStencil(targ->getInfo().depthFormat);

					if(ctx.currDepth.enableDepthWrite != true)
						glDepthMask(ctx.currDepth.enableDepthWrite = true);

					//TODO: Test stencil buffers, since they might need stencil write to be turned on

					if(depth && stencil)
						glClearNamedFramebufferfi(dat->index, GL_DEPTH_STENCIL, 0, ctx.depth, ctx.stencil);

					else {

						if(depth)
							glClearNamedFramebufferfv(dat->index, GL_DEPTH, 0, &ctx.depth);

						if (stencil)
							glClearNamedFramebufferiv(dat->index, GL_STENCIL, 0, (GLint *)&ctx.stencil);
					}
				}

				if (cf->clearFlags & ClearFramebuffer::COLOR) {

					for (GLint i = 0, j = GLint(targ->size()); i < j; ++i) {

						if (ctx.clearColor.type == SetClearColor::Type::FLOAT)
							glClearNamedFramebufferfv(dat->index, GL_COLOR, i, ctx.clearColor.rgbaf.arr);
						else if (ctx.clearColor.type == SetClearColor::Type::UNSIGNED_INT)
							glClearNamedFramebufferuiv(dat->index, GL_COLOR, i, ctx.clearColor.rgbau.arr);
						else
							glClearNamedFramebufferiv(dat->index, GL_COLOR, i, ctx.clearColor.rgbai.arr);
					}
				}

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

				ctx.currentFramebuffer->end();
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

				auto *pbuffer = ((BindPrimitiveBuffer*) c)->bindObject;

				if (pbuffer != ctx.primitiveBuffer) {

					ctx.primitiveBuffer = pbuffer;

					if (ctx.vaos.find(pbuffer) == ctx.vaos.end())
						ctx.vaos[pbuffer] = glxGenerateVao(pbuffer);

					GLuint vao = ctx.vaos[pbuffer];
					glBindVertexArray(vao);
				}

				break;
			}

			case CMD_BIND_PIPELINE: {

				auto *pipeline = ((BindPipeline*) c)->bindObject;

				if (pipeline != ctx.pipeline) {
					ctx.pipeline = pipeline;
					glxBindPipeline(ctx, pipeline);
				}

				break;
			}

			case CMD_BIND_DESCRIPTORS: {

				auto *descriptors = ((BindDescriptors*) c)->bindObject;

				if (descriptors != ctx.descriptors) {

					ctx.descriptors = descriptors;

					if(descriptors)
						glxBindDescriptors(ctx, descriptors);
				}

				break;
			}

			case CMD_DRAW_INSTANCED:

				if (!ctx.primitiveBuffer)
					oic::System::log()->fatal("No primitive buffer bound!");

				if (!ctx.pipeline)
					oic::System::log()->fatal("No pipeline bound!");

				if (!ctx.pipeline->isGraphics())
					oic::System::log()->fatal("Pipeline bound was invalid; graphics expected");

				if(!ctx.primitiveBuffer->matchLayout(
					ctx.pipeline->getInfo().attributeLayout
				))
					oic::System::log()->fatal("Pipeline vertex layout doesn't match primitive buffer!");

				if (ctx.descriptors && 
					!ctx.descriptors->isShaderCompatible(
						ctx.pipeline->getInfo().pipelineLayout
					)
				)
					oic::System::log()->fatal("Pipeline layout doesn't match descriptors!");

				if(!ctx.currentFramebuffer)
					oic::System::log()->fatal("No surface bound");

				if(
					ctx.currentFramebuffer->getInfo().samples != 
					ctx.pipeline->getInfo().msaa.samples
				)
					oic::System::log()->fatal("Surface didn't have the same number of samples as pipeline");

				{
					auto topo = glxTopologyMode(ctx.pipeline->getInfo().topology);
					auto *di = (DrawInstanced*) c;

					if (di->isIndexed)
						glDrawElementsInstancedBaseVertexBaseInstance(
							topo,
							di->count,
							glxGpuFormatType(ctx.primitiveBuffer->getIndexFormat()),
							(void*) (
										usz(di->start) * 
										FormatHelper::getSizeBytes(
											ctx.primitiveBuffer->getIndexFormat()
										)
									 ),
							di->instanceCount,
							di->vertexStart,
							di->instanceStart
						);
					else
						glDrawArraysInstancedBaseInstance(
							topo,
							di->start,
							di->count,
							di->instanceCount,
							di->instanceStart
						);
				}

				break;

			case CMD_DISPATCH:

				if (!ctx.pipeline)
					oic::System::log()->fatal("No pipeline bound!");

				if (!ctx.pipeline->isCompute())
					oic::System::log()->fatal("Pipeline bound was invalid; compute expected");

				{
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
				}

				break;


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
				oic::System::log()->fatal("Unsupported operation");

		}

	}

}