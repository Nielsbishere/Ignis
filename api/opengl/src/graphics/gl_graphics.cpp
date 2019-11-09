#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
#include "graphics/gl_graphics.hpp"
#include "graphics/surface/gl_framebuffer.hpp"
#include "graphics/surface/swapchain.hpp"

namespace ignis {

	Graphics::~Graphics() { delete data; }

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

		for (CommandList *cl : commands)
			cl->execute();
	}

	void Graphics::present(
		Framebuffer *intermediate, Swapchain *swapchain,
		const List<CommandList*> &commands
	) {

		if (!intermediate || !swapchain || !commands.size())
			oic::System::log()->fatal("Couldn't present; invalid intermediate or swapchain");

		if(intermediate->getInfo().size != swapchain->getInfo().size)
			oic::System::log()->fatal("Couldn't present; swapchain and intermediate aren't same size");

		execute(commands);

		Vec2u size = intermediate->getInfo().size;

		glBlitNamedFramebuffer(
			data->bound[GL_READ_FRAMEBUFFER] = intermediate->getData()->index,
			data->bound[GL_DRAW_FRAMEBUFFER] = 0,
			0, 0, size[0], size[1],
			0, 0, size[0], size[1],
			GL_COLOR_BUFFER_BIT, GL_NEAREST
		);

		swapchain->present();
	}

}