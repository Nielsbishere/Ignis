#pragma once
#include "error/ignis.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "graphics/graphics_object.hpp"
#define __impl

namespace ignis {

	class Graphics;

	//A struct that can be inherrited; size should store the size of the struct
	//Can store the pointers necessary for executing the operation, but size can also be 0 if no additional data is needed
	//op has to be unique since it is used as an identifier
	struct Command {

		u32 op, size;

		Command(u32 op, usz size): op(op), size(u32(size)) {
			if (size >= u32_MAX)
				oic::System::log()->fatal(errors::commands::tooBig);
		}
	};

	//Storing a list of GPU commands that can be executed by the interface
	class CommandList : public GraphicsObject {

	public:

		struct Info {

			usz bufferSize;
			Info(usz bufferSize): bufferSize(bufferSize) {}
		};

		struct Data {

			Buffer commandBuffer;
			usz next{};

			Data(const Info &info): commandBuffer(info.bufferSize) {}
		};

		CommandList(Graphics &g, const Info &info);		//Construct new command list
		CommandList(Graphics &g, const Data &data);		//Copy old command list
		~CommandList() = default;

		const Info &getInfo() const;
		Data &getData();

		void clear();
		void resize(usz newSize);

		//Add commands to the command list

		template<typename ...args>
		inline void add(const args &...arg);

		template<typename T>
		inline void add(const T &t);

		template<typename ...args>
		inline void add(const args *...arg);

		template<typename T, typename = std::enable_if<std::is_base_of_v<Command, T> && std::is_pod_v<T> && sizeof(T) <= u16_MAX>>
		inline void add(const T *t);

		void execute();

	protected:

		void addInternal(const Command *c);
		__impl void execute(Command *c);

	private:

		Info info;
		Data data;
	};

	template<typename ...args>
	void CommandList::add(const args &...arg) {
		(add(arg), ...);
	}

	template<typename T>
	void CommandList::add(const T &t) {
		add(&t);
	}

	template<typename ...args>
	void CommandList::add(const args *...arg) {
		(add(arg), ...);
	}

	template<typename T, typename>
	void CommandList::add(const T *t) {
		addInternal((const Command*)t);
	}

}