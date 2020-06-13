#include "graphics/command/command_list.hpp"
#include "graphics/graphics.hpp"
#include "system/system.hpp"
#include <cstring>

namespace ignis {

	void CommandList::clear() {

		for (Command *c : info.commands)
			c->~Command();

		info.next = 0;
		info.commands.clear();
	}

	void CommandList::resize(usz newSize) {

		if (newSize < info.next)
			oic::System::log()->fatal("The command list resize would result into loss of data");

		info.commandBuffer.resize(newSize);
	}
}