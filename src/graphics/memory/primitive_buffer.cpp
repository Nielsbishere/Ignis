#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/gpu_buffer.hpp"
#include "utils/hash.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace ignis {

	PrimitiveBuffer::PrimitiveBuffer(
		Graphics &g, const String &name, const Info &inf
	):
		GraphicsObject(g, name), info(inf)
	{
		usz i{}, elements{};

		for (auto &it : info.vertexLayout) {

			if (!it.buffer) {

				it.buffer = new GPUBuffer(g, NAME(name + " vbo " + std::to_string(i)),
					GPUBuffer::Info(
						it.initData,
						GPUBufferType::VERTEX,
						info.usage
					)
				);

				it.initData.clear();

				if(!it.size())
					oic::System::log()->fatal("Invalid primitive buffer size");

				if (!elements)
					elements = it.elements;

				else if(elements != it.elements)
					oic::System::log()->fatal("Invalid primitive buffer size");

			} else if(it.buffer->getInfo().type != GPUBufferType::VERTEX)
				oic::System::log()->fatal("Invalid predefined vertex buffer");

			else {

				if (!elements)
					elements = it.elements;

				if (elements != it.elements || !elements)
					oic::System::log()->fatal("Invalid predefined vertex buffer size");
			}

			if (it.buffer->getInfo().size != it.size())
				oic::System::log()->fatal("Invalid primitive buffer size");

			++i;
		}

		if(info.vertexLayout.size() == 0)
			oic::System::log()->fatal("Primitive buffer requires at least one vertex buffer");

		if (isIndexed()) {

			if (!info.indexLayout.buffer) {

				info.indexLayout.buffer = new GPUBuffer(g, NAME(name + " ibo"),
					GPUBuffer::Info(
						info.indexLayout.initData,
						GPUBufferType::INDEX,
						info.usage
					)
				);

				info.indexLayout.initData.clear();

			} else if (info.indexLayout.buffer->getInfo().type != GPUBufferType::INDEX)
				oic::System::log()->fatal("Invalid predefined index buffer");
		}

		init();
	}

	PrimitiveBuffer::~PrimitiveBuffer() {

		destroy();

		for (auto &elem : info.vertexLayout)
			::destroy(elem.buffer);

		::destroy(info.indexLayout.buffer);
	}
}