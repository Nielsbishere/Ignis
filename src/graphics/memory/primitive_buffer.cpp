#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/gpu_buffer.hpp"
#include "utils/hash.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace ignis {

	BufferLayout::BufferLayout(GPUBuffer *b, const BufferAttributes &formats, usz bufferOffset) :
		buffer(b), formats(formats), elements(u32(b->size() / formats.getStride())),
		bufferOffset(bufferOffset) {}

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

			if (it.buffer->size() != it.size())
				oic::System::log()->fatal("Invalid primitive buffer size");

			it.buffer->addRef();

			++i;
		}

		if(info.vertexLayout.size() == 0)
			oic::System::log()->fatal("Primitive buffer requires at least one vertex buffer");

		if (isIndexed()) {

			if (!info.indexLayout.buffer) {

				if(info.indexLayout.formats.size() != 1)
					oic::System::log()->fatal("Index buffer requires one format");

				info.indexLayout.buffer = new GPUBuffer(
					g, NAME(name + " ibo"),
					GPUBuffer::Info(
						info.indexLayout.initData,
						GPUBufferType::INDEX,
						info.usage
					)
				);

				info.indexLayout.initData.clear();

			} else if (info.indexLayout.buffer->getInfo().type != GPUBufferType::INDEX)
				oic::System::log()->fatal("Invalid predefined index buffer");

			else
				info.indexLayout.buffer->addRef();
		}
	}

	PrimitiveBuffer::~PrimitiveBuffer() {

		for (auto &elem : info.vertexLayout)
			elem.buffer->loseRef();

		if(GPUBuffer *buf = info.indexLayout.buffer)
			buf->loseRef();
	}

	const bool PrimitiveBuffer::matchLayout(const List<BufferAttributes> &layout) const {

		if (layout.size() != info.vertexLayout.size())
			return false;

		for (usz i = 0, j = layout.size(); i < j; ++i)
			if (layout[i] != info.vertexLayout[i].formats)
				return false;

		return true;
	}
}