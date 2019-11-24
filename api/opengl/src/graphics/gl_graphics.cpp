#include "utils/thread.hpp"
#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/gl_graphics.hpp"
#include "graphics/gl_context.hpp"
#include "graphics/surface/gl_framebuffer.hpp"
#include "graphics/surface/swapchain.hpp"
#include "system/system.hpp"

namespace ignis {

	Graphics::~Graphics() { 
		release();
		delete data;
	}

	Graphics::Graphics() { 
		data = new Graphics::Data(); 
		init();
	}

	GraphicsApi Graphics::getCurrentApi() const {
		return GraphicsApi::OPENGL;
	}

	CommandAvailability Graphics::getCommandAvailability(CommandOp op) {
		
		if (op >> CMD_PROPERTY_TECHNIQUE_SHIFT)
			return CommandAvailability::UNSUPPORTED;

		return CommandAvailability::SUPPORTED;
	}

	void Graphics::execute(const List<CommandList*> &commands) {

		//Updates VAOs and FBOs that have been added/released
		data->updateContext();

		for (CommandList *cl : commands)
			cl->execute();
	}

	void Graphics::present(
		Framebuffer *intermediate, Swapchain *swapchain,
		const List<CommandList*> &commands
	) {

		if (!swapchain)
			oic::System::log()->fatal("Couldn't present; invalid intermediate or swapchain");

		if(!intermediate)
			oic::System::log()->warn("Presenting without an intermediate is valid but won't provide any results to the swapchain");

		if(intermediate && intermediate->getInfo().size != swapchain->getInfo().size)
			oic::System::log()->fatal("Couldn't present; swapchain and intermediate aren't same size");

		GLContext &ctx = data->getContext();

		swapchain->bind();

		execute(commands);

		if (intermediate) {

			Vec2u size = intermediate->getInfo().size;

			glBlitNamedFramebuffer(
				ctx.bound[GL_READ_FRAMEBUFFER] = intermediate->getData()->index,
				ctx.bound[GL_DRAW_FRAMEBUFFER] = 0,
				0, 0, size[0], size[1],
				0, 0, size[0], size[1],
				GL_COLOR_BUFFER_BIT, GL_LINEAR
			);
		}

		swapchain->present();
	}

	//Keep track of objects for updating gl contexts

	void Graphics::onAddOrErase(GraphicsObject *go, bool isDeleted) {

		if (go->canCast<PrimitiveBuffer>()) {

			auto *pb = (PrimitiveBuffer*)go;

			if(!isDeleted)
				data->primitiveBuffers[pb];
			else
				data->primitiveBuffers.erase(pb);

			//Remove all referenced VAOs in contexts next time they update

			for (auto &context : data->contexts)
				if(context.second.vaos.find(pb) != context.second.vaos.end())
					context.second.deletedVaos.push_back(pb);
		}

	}

	void Graphics::Data::updateContext() {

		GLContext &context = getContext();

		//Clean up left over VAOs

		for (auto &del : context.deletedVaos) {
			glDeleteVertexArrays(1, &context.vaos[del]);
			context.vaos.erase(del);
		}

		context.deletedVaos.clear();

	}

	void Graphics::Data::destroyContext() {

		GLContext &context = getContext();
		
		for(auto &vao : context.vaos)
			glDeleteVertexArrays(1, &vao.second);

		contexts.erase(oic::Thread::getCurrentId());
	}

	GLContext &Graphics::Data::getContext() {
		return contexts[oic::Thread::getCurrentId()];
	}

}