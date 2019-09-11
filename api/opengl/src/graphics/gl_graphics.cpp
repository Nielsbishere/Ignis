#include "graphics/command/command_list.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	Graphics::Graphics() { data = new Graphics::Data(); }
	Graphics::~Graphics() { delete data; }

	CommandAvailability Graphics::getCommandAvailability(CommandOp) {

		//TODO:

		return CommandAvailability::SUPPORTED;
	}

	void Graphics::execute(const List<CommandList*> &commands) {

		for (CommandList *cl : commands)
			cl->execute();
	}

}