#pragma once
#include "errors/ignis.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace ignis {

	class Graphics;

	//A struct that can be inherrited; size should store the size of the struct
	//Can store the pointers necessary for executing the operation, but size can also be 0 if no additional data is needed
	//op has to be unique since it is used as an identifier
	//Custom commands have to start at code 0x80000000 (isCustom)
	struct Command {

		static constexpr u32 isCustom = 0x80000000;

		u32 op, size;

		Command(u32 op, usz size): op(op), size(size - sizeof(Command)) {
			if (size - sizeof(Command) >= u32_MAX)
				oic::System::log()->fatal(errors::commands::tooBig);
		}
	};

	//Storing a list of GPU commands that can be executed by the interface
	class CommandList {

	public:

		struct Info {

			u32 bufferSize;
			Info(u32 bufferSize): bufferSize(bufferSize) {}
		};

		struct Data {

			Buffer commandBuffer;
			usz next{};

			Data(const Info &info): commandBuffer(info.bufferSize) {}
		};

		CommandList(const Info &info);		//Construct new command list
		CommandList(const Data &data);		//Copy old command list
		~CommandList() = default;

		CommandList(const CommandList&) = delete;
		CommandList(CommandList&&) = delete;
		CommandList &operator=(const CommandList&) = delete;
		CommandList &operator=(CommandList&&) = delete;

		const Info &getInfo() const;
		Data &getData();

		void clear();
		void resize(usz newSize);

		//Add commands to the command list

		template<typename ...args>
		void add(const args &...arg);

		template<typename T>
		void add(const T &t);

		template<typename ...args>
		void add(const args *...arg);

		template<typename T, typename = std::enable_if<std::is_base_of_v<Command, T> && std::is_pod_v<T> && sizeof(T) <= u16_MAX>>
		void add(const T *t);

		void execute(Graphics &g) const;

	protected:

		virtual void execute(Graphics &g, Command *c) const = 0;

	private:

		Info info;
		Data data;
	};

	template<typename ...args>
	void CommandList::add(const args &...arg) {
		add(arg)...;
	}

	template<typename T>
	void CommandList::add(const T &t) {
		add(&t);
	}

	template<typename ...args>
	void CommandList::add(const args *...arg) {
		add(arg)...;
	}

	template<typename T, typename>
	void CommandList::add(const T *t) {
		
		usz size = sizeof(Command) + t->size;

		if(size < sizeof(T))
			oic::System::log()->fatal(errors::commands::insufficientData);

		if(data.next + size >= info.bufferSize)
			oic::System::log()->fatal(errors::commands::outOfBounds);

		memcpy(data.commandBuffer.data() + data.next, t, size);
		data.next += size;
	}

}