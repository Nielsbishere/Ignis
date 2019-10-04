#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
#include "graphics/command/commands.hpp"
#include "graphics/surface/swapchain.hpp"
#include "graphics/memory/gl_primitive_buffer.hpp"
#include "graphics/surface/gl_framebuffer.hpp"
#include "graphics/shader/pipeline.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	void CommandList::execute(Command *c) {

		using namespace cmd;

		Graphics::Data &gdata = *getGraphics().getData();

		//TODO: Instead of doing validation here; do it whenever a command enters the command buffer!

		switch (c->op) {

			case CMD_BEGIN_SURFACE:
				{
					auto *bs = (BeginSurface*)c;
					bs->bindObject->begin(bs->renderArea);
					gdata.currentSurface = bs->bindObject;
				}
				break;

			case CMD_END_SURFACE:

				gdata.currentSurface->end();
				break;

			case CMD_PRESENT:

				gdata.swapchain->present();
				break;

			case CMD_DEBUG_START_REGION:

				if (glPushDebugGroup) {

					auto *sr = (DebugStartRegion*)c;

					glPushDebugGroup(
						GL_DEBUG_SOURCE_APPLICATION,
						0,
						GLsizei(sr->size()),
						sr->string
					);
				}

				break;

			case CMD_DEBUG_END_REGION:

				if (glPopDebugGroup)
					glPopDebugGroup();

				break;

			case CMD_DEBUG_INSERT_MARKER:

				if (glDebugMessageInsert) {

					auto *im = (DebugInsertMarker*)c;

					glDebugMessageInsert(
						GL_DEBUG_SOURCE_APPLICATION,
						GL_DEBUG_TYPE_MARKER,
						0,
						GL_DEBUG_SEVERITY_NOTIFICATION,
						GLsizei(im->size()),
						im->string
					);
				}

				break;

			case CMD_SET_CLEAR_COLOR:

				{
					SetClearColor *cc = (SetClearColor*)c;
					Vec4f color = cc->rgbaf;

					if (cc->type == SetClearColor::Type::UNSIGNED_INT)
						color = Vec4f{ 
							f32(cc->rgbau[0]), 
							f32(cc->rgbau[1]),
							f32(cc->rgbau[2]),
							f32(cc->rgbau[3]) 
						};

					else if(cc->type == SetClearColor::Type::SIGNED_INT)
						color = Vec4f {
							f32(cc->rgbai[0]),
							f32(cc->rgbai[1]),
							f32(cc->rgbai[2]),
							f32(cc->rgbai[3])
						};

					if (color != gdata.clearColor) {
						glClearColor(color[0], color[1], color[2], color[3]);
						gdata.clearColor = color;
					}
				}
				break;

			case CMD_SET_CLEAR_DEPTH:

				{
					SetClearDepth *cd = (SetClearDepth*)c;

					if (cd->dataObject != gdata.depth)
						glClearDepth(gdata.depth = cd->dataObject);
				}
				break;

			case CMD_SET_CLEAR_STENCIL:

				{
					SetClearStencil *cd = (SetClearStencil*)c;

					if (cd->dataObject != gdata.stencil)
						glClearDepth(gdata.stencil = cd->dataObject);
				}
				break;

			case CMD_BLIT_SURFACE:

				{
					BlitSurface *bs = (BlitSurface*) c;

					GLenum mask{};
					GLenum filter = bs->filter == BlitSurface::BlitFilter::NEAREST ? GL_NEAREST : GL_LINEAR;

					if (bs->mask & BlitSurface::COLOR)
						mask |= GL_COLOR_BUFFER_BIT;
					else if (bs->mask & BlitSurface::DEPTH)
						mask |= GL_DEPTH_BUFFER_BIT;
					else
						mask |= GL_STENCIL_BUFFER_BIT;

					GLuint index = 0; 

					if (bs->src->canCast<Framebuffer>())
						index = ((Framebuffer*) bs->src)->getData()->index;

					gdata.bind(glBindFramebuffer, GL_READ_FRAMEBUFFER, index);

					index = 0;

					if (bs->dst->canCast<Framebuffer>())
						index = ((Framebuffer*) bs->dst)->getData()->index;

					gdata.bind(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, index);

					Vec4u srcArea = bs->srcArea, dstArea = bs->dstArea;

					if (!srcArea[2]) srcArea[2] = bs->src->getInfo().size[0];
					if (!srcArea[3]) srcArea[3] = bs->src->getInfo().size[1];
					if (!dstArea[2]) dstArea[2] = bs->dst->getInfo().size[0];
					if (!dstArea[3]) dstArea[3] = bs->dst->getInfo().size[1];

					glBlitFramebuffer(
						srcArea[0], srcArea[1], srcArea[2], srcArea[3],
						dstArea[0], dstArea[1], dstArea[2], dstArea[3],
						mask,
						filter
					);

				}
				break;

			case CMD_BIND_PRIMITIVE_BUFFER:
				
				{
					auto *pbuffer = ((BindPrimitiveBuffer*) c)->bindObject;

					if (pbuffer != gdata.primitiveBuffer) {
						gdata.primitiveBuffer = pbuffer;
						glBindVertexArray(pbuffer->getData()->handle);
					}
				}
				break;

			case CMD_BIND_PIPELINE:
				
				{
					auto *pipeline = ((BindPipeline*) c)->bindObject;

					if (pipeline != gdata.pipeline) {
						gdata.pipeline = pipeline;
						glBindPipeline(gdata, pipeline);
					}
				}
				break;

			case CMD_DRAW_INSTANCED:

				if (!gdata.primitiveBuffer)
					oic::System::log()->fatal("No primitive buffer bound!");

				if (!gdata.pipeline)
					oic::System::log()->fatal("No pipeline bound!");

				if(!gdata.primitiveBuffer->matchLayout(gdata.pipeline->getInfo().attributeLayout))
					oic::System::log()->fatal("Pipeline vertex layout doesn't match primitive buffer!");

				{
					auto topo = glTopologyMode(gdata.pipeline->getInfo().topology);
					auto *di = (DrawInstanced*) c;

					if (gdata.primitiveBuffer->indices())
						glDrawElementsInstanced(
							topo,
							di->count,
							glGpuFormat(gdata.primitiveBuffer->getIndexFormat()),
							(void*) usz(di->start),
							di->instanceCount
						);
					else
						glDrawArraysInstanced(
							topo,
							di->start,
							di->count,
							di->instanceCount
						);
				}

				break;

			default:
				oic::System::log()->fatal("Unsupported operation");

		}

	}

}