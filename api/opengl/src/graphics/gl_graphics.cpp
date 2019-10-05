#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	Graphics::~Graphics() { delete data; }

	Graphics::Graphics() { 
		data = new Graphics::Data(); 
		init();
	}

	GraphicsApi Graphics::getCurrentApi() const {
		return GraphicsApi::OPENGL;
	}

	CommandAvailability Graphics::getCommandAvailability(CommandOp) {
		//TODO:
		return CommandAvailability::SUPPORTED;
	}

	void Graphics::execute(const List<CommandList*> &commands) {

		for (CommandList *cl : commands)
			cl->execute();
	}

}