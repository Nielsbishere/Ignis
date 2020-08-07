#pragma once
#include "system/log.hpp"
#include "graphics/graphics.hpp"

namespace ignis {

	class Graphics;
	class Command;

	//Storing a list of GPU commands that can be executed by the interface
	class CommandList : public GPUObject {

		friend class Graphics;

	public:

		struct Info {

			usz bufferSize;

			Buffer commandBuffer;
			usz next{};

			List<Command*> commands;

			Info(usz bufferSize): bufferSize(bufferSize), commandBuffer(bufferSize) {}

			/*
			~Info() {
				bufferSize = next = 0;
				commandBuffer.clear();
				commands.clear();
			}

			Info(const Info&);
			Info(Info&&);
			Info &operator=(const Info&);
			Info &operator=(Info&&);
			*/
		};

		apimpl struct Data;

		apimpl CommandList(Graphics &g, const String &name, const Info &info);

		inline const Info &getInfo() const { return info; }
		inline Data *getData() const { return data; }

		void clear();
		void resize(usz newSize);

		inline bool empty() const { return !info.next; }

		//Add commands to the command list

		template<typename T, typename ...args>
		inline void add(const T &t, const args &...arg);

	protected:

		apimpl void execute(List<GPUObject*> &resources);	//Returns resources used by this execution

	private:

		apimpl ~CommandList();

		Info info;
		Data *data{};
	};

	//A copyable struct that can be placed inside of a command buffer
	//
	class Command {

		friend class CommandList;

		virtual void prepare(Graphics&, CommandList::Data*) {}
		virtual void execute(Graphics&, CommandList::Data*) const = 0;
		virtual List<GPUObject*> getResources() const { return {}; }

		//TODO:
		//virtual void onCopy(const Command *c) = 0;
		//virtual void onMove(Command *c) = 0;

	protected:
		virtual ~Command() = default;

	public:
		Command() = default;

	};

	//Classification for debug commands
	class DebugCommand : public Command {};

	//A command that relies on a feature to be present
	//
	class CommandFt : public Command {

		Feature feature;

	protected:

		CommandFt(Feature ft): feature(ft) {}

	public:

		inline Feature getFeature() const { return feature; }
	};

	//A command that relies on an extension to be present
	//
	class CommandExt : public Command {

		Extension extension;

	protected:

		CommandExt(Extension ext): extension(ext) {}

	public:

		inline Extension getExtension() const { return extension; }
	};

	//Implementations

	template<typename T, typename ...args>
	inline void CommandList::add(const T &t, const args &...arg) {

		static_assert(std::is_base_of_v<Command, T>, "CommandList::add requires T to be a command");

		constexpr usz size = sizeof(T);

		if (info.next + size > info.bufferSize)
			oic::System::log()->fatal("The command list couldn't hold the commands");

		auto *addr = info.commandBuffer.data() + info.next;

		::new(addr) T(t);
		info.next += size;
		info.commands.push_back((Command*)addr);

		if constexpr(sizeof...(arg) > 0)
			(add(arg), ...);
	}

	using CommandListRef = GraphicsObjectRef<CommandList>;

}