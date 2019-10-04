#pragma once
#include "../graphics_object.hpp"
#include "graphics/memory/buffer_layout.hpp"

namespace ignis {

	enum class TopologyMode : u8;

	class Pipeline : public GraphicsObject {

	public:

		enum class Flag : u8 {
			OPTIMIZE = 1 << 0
		};

		struct Info {

			List<BufferAttributes> attributeLayout;
			Flag flag;
			TopologyMode topology;

			Info(Flag f, const List<BufferAttributes> &attributeLayout, TopologyMode topology) : 
				flag(f), attributeLayout(attributeLayout), topology(topology) { }

		};

		apimpl struct Data;

		apimpl Pipeline(Graphics &g, const String &name, const Info &info);
		apimpl ~Pipeline();

		const Info &getInfo() const { return info; }
		Data *getData() { return data; }

	private:

		Info info;
		Data *data{};
	};

}