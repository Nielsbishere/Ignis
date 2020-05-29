#pragma once
#include "system/log.hpp"
#include "graphics/graphics.hpp"

namespace ignis {

	class Graphics;

	//A struct that can be inherrited; size should store the size of the struct
	//Can store the pointers necessary for executing the operation, but size can also be 0 if no additional data is needed
	//op has to be unique since it is used as an identifier
	struct Command {

		u32 op, commandSize;

		Command(u32 op, usz commandSize);
	};

	//Storing a list of GPU commands that can be executed by the interface
	class CommandList : public GPUObject {

		friend class Graphics;

	public:

		struct Info {

			usz bufferSize;

			Buffer commandBuffer;
			usz next{};

			Info(usz bufferSize): bufferSize(bufferSize), commandBuffer(bufferSize) {}
		};

		apimpl struct Data;

		apimpl CommandList(Graphics &g, const String &name, const Info &info);

		inline const Info &getInfo() const { return info; }
		inline Data *getData() const { return data; }

		void clear();
		void resize(usz newSize);

		inline bool empty() const { return !info.next; }

		//Add commands to the command list

		template<typename ...args>
		inline void add(const args &...arg);

		template<typename T>
		inline void add(const T &t);

		template<typename ...args>
		inline void add(const args *...arg);

		template<typename T, typename = std::enable_if<std::is_base_of_v<Command, T> && std::is_pod_v<T> && sizeof(T) <= u16_MAX>>
		inline void add(const T *t);

		apimpl void execute();

	protected:

		void addInternal(const Command *c);
		apimpl void execute(Command *c);

	private:

		apimpl ~CommandList();

		Info info;
		Data *data{};
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