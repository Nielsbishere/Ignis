#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
#include "graphics/command/commands.hpp"
#include "graphics/surface/surface.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	void CommandList::execute(Command *c) {

		using namespace cmd;

		Graphics::Data &gdata = *getGraphics().getData();

		switch (c->op) {

			//Binding

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

			//Clearing

			case CMD_SET_CLEAR_COLOR:

				{
					SetClearColor *cc = (SetClearColor*)c;
					Vec4f color = cc->rgbaf;

					if (cc->type == SetClearColor::UNSIGNED_INT)
						color = Vec4f{ 
							f32(cc->rgbau[0]), 
							f32(cc->rgbau[1]),
							f32(cc->rgbau[2]),
							f32(cc->rgbau[3]) 
						};

					else if(cc->type == SetClearColor::SIGNED_INT)
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

			default:
				oic::System::log()->fatal("Unsupported operation");

		}

	}

}