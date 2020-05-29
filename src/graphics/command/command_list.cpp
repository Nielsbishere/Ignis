#include "graphics/command/command_list.hpp"
#include "graphics/graphics.hpp"
#include "system/system.hpp"
#include <cstring>

namespace ignis {

	Command::Command(u32 op, usz commandSize): op(op), commandSize(u32(commandSize)) {
		if (commandSize >= u32_MAX)
			oic::System::log()->fatal("The command contained too much data");
	}

	void CommandList::clear() {
		info.next = 0;
	}

	void CommandList::resize(usz newSize) {

		if (newSize < info.next)
			oic::System::log()->fatal("The command list resize would result into loss of data");

		info.commandBuffer.resize(newSize);
	}

	void CommandList::addInternal(const Command *c) {

		auto availability = getGraphics().getCommandAvailability(CommandOp(c->op));

		if (availability == CommandAvailability::PERFORMANCE)
			oic::System::log()->performance("The command is not natively supported and could lead to performance loss");

		else if (availability == CommandAvailability::UNSUPPORTED) {
			oic::System::log()->fatal("The command is not supported and will be ignored");
			return;
		}

		usz size = c->commandSize;

		if (info.next + size > info.bufferSize)
			oic::System::log()->fatal("The command list couldn't hold the commands");

		std::memcpy(info.commandBuffer.data() + info.next, c, size);
		info.next += size;
	}

}