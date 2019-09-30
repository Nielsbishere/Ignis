#include "graphics/command/command_list.hpp"
#include "graphics/gl_graphics.hpp"

namespace ignis {

	Graphics::~Graphics() { delete data; }

	Graphics::Graphics() { 

		data = new Graphics::Data(); 
		init();

		setFeature(Feature::STORAGE_BUFFER, !data->isES && data->version(4, 3));
		setFeature(Feature::COMPUTE, !data->isES && data->version(4, 3));

		setExtension(Extension::DRAW_INDIRECT, !data->isES && data->version(4, 0));
		setExtension(Extension::DISPATCH_INDIRECT, !data->isES && data->version(4, 3));
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