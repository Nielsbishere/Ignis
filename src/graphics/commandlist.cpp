#include "graphics/commandlist.hpp"

namespace ignis {

	CommandList::CommandList(const Info &info): info(info), data(info) {}
	CommandList::CommandList(const Data &data) : info(u32(data.commandBuffer.size())), data(data) {}

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

	void CommandList::execute(Graphics &g) const {

		for (u8 *ptr = (u8*)data.commandBuffer.data(), *end = ptr + data.next; ptr != end; ) {
			Command *c = (Command*)ptr;
			execute(g, c);
			ptr += c->size;
		}
	}

}