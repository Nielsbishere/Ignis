#include "graphics/command/command_list.hpp"
#include "graphics/graphics.hpp"

namespace ignis {

	CommandList::CommandList(Graphics &g, const Info &info):
		GraphicsObject(g), info(info), data(info) {}

	CommandList::CommandList(Graphics &g, const Data &data):
		GraphicsObject(g), info(u32(data.commandBuffer.size())), data(data) {}

	const CommandList::Info &CommandList::getInfo() const { return info; }
	CommandList::Data &CommandList::getData() { return data; }

	void CommandList::clear() {
		data.next = 0;
	}

	void CommandList::resize(usz newSize) {

		if (newSize < data.next)
			oic::System::log()->fatal(errors::commands::lossOfData);

		data.commandBuffer.resize(newSize);
	}

	void CommandList::execute() {

		for (u8 *ptr = (u8*)data.commandBuffer.data(), *end = ptr + data.next; ptr < end; ) {
			Command *c = (Command*)ptr;
			execute(c);
			ptr += c->size;
		}
	}

	void CommandList::addInternal(const Command *c) {

		auto availability = getGraphics().getCommandAvailability(CommandOp(c->op));

		if (availability == CommandAvailability::PERFORMANCE)
			oic::System::log()->performance(errors::commands::performanceLoss);
		else if (availability == CommandAvailability::UNSUPPORTED)
			oic::System::log()->fatal(errors::commands::notSupported);

		usz size = c->size;

		if (data.next + size >= info.bufferSize)
			oic::System::log()->fatal(errors::commands::outOfBounds);

		memcpy(data.commandBuffer.data() + data.next, c, size);
		data.next += size;
	}

}