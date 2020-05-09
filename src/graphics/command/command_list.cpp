#include "graphics/command/command_list.hpp"
#include "graphics/graphics.hpp"
#include "system/system.hpp"
#include <cstring>

namespace ignis {

	Command::Command(u32 op, usz size): op(op), size(u32(size)) {
		if (size >= u32_MAX)
			oic::System::log()->fatal("The command contained too much data");
	}

	CommandList::CommandList(Graphics &g, const String &name, const Info &info):
		GPUObject(g, name, GPUObjectType::COMMAND_LIST), info(info), data(info) {}

	CommandList::CommandList(Graphics &g, const String &name, const Data &data):
		GPUObject(g, name, GPUObjectType::COMMAND_LIST), info(u32(data.commandBuffer.size())), data(data) {}

	const CommandList::Info &CommandList::getInfo() const { return info; }
	CommandList::Data &CommandList::getData() { return data; }

	void CommandList::clear() {
		data.next = 0;
	}

	void CommandList::resize(usz newSize) {

		if (newSize < data.next)
			oic::System::log()->fatal("The command list resize would result into loss of data");

		data.commandBuffer.resize(newSize);
	}

	void CommandList::addInternal(const Command *c) {

		auto availability = getGraphics().getCommandAvailability(CommandOp(c->op));

		if (availability == CommandAvailability::PERFORMANCE)
			oic::System::log()->performance("The command is not natively supported and could lead to performance loss");

		else if (availability == CommandAvailability::UNSUPPORTED) {
			oic::System::log()->fatal("The command is not supported and will be ignored");
			return;
		}

		usz size = c->size;

		if (data.next + size >= info.bufferSize)
			oic::System::log()->fatal("The command list couldn't hold the commands");

		std::memcpy(data.commandBuffer.data() + data.next, c, size);
		data.next += size;
	}

}