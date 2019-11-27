#pragma once
#include "../graphics_object.hpp"
#include "buffer_layout.hpp"

namespace ignis {

	class PrimitiveBuffer : public GraphicsObject {

	public:

		struct Info {

			List<BufferLayout> vertexLayout;
			BufferLayout indexLayout;
			GPUMemoryUsage usage;

			Info(
				const List<BufferLayout> &vertexLayout, 
				const BufferLayout &indexLayout = {},
				GPUMemoryUsage usage = GPUMemoryUsage::LOCAL
			):
				vertexLayout(vertexLayout), indexLayout(indexLayout), usage(usage) { }

			Info(
				const BufferLayout &vertexLayout, 
				const BufferLayout &indexLayout = {},
				GPUMemoryUsage usage = GPUMemoryUsage::LOCAL
			):
				vertexLayout{ vertexLayout }, indexLayout(indexLayout), usage(usage) { }
		};

		apimpl struct Data;

		PrimitiveBuffer(Graphics &g, const String &name, const Info &info);
		~PrimitiveBuffer();

		inline const BufferLayout &operator[](usz i) const;
		inline const BufferLayout &getIndexBuffer() const;
		inline const bool isIndexed() const;
		inline const GPUMemoryUsage getUsage() const;
		inline const u32 indices() const;
		inline const u32 vertices() const;
		inline const u32 elements() const;
		inline const GPUFormat getIndexFormat() const;

		inline Data *getData() { return data; }
		inline const Info &getInfo() const { return info; }

		//Whether or not this primitive buffer matches a layout
		const bool matchLayout(const List<BufferAttributes> &layout) const;

	private:

		Info info;
		Data *data{};
	};

	inline const BufferLayout &PrimitiveBuffer::operator[](usz i) const {
		return info.vertexLayout[i];
	}

	inline const BufferLayout &PrimitiveBuffer::getIndexBuffer() const {
		return info.indexLayout;
	}

	inline const bool PrimitiveBuffer::isIndexed() const {
		return info.indexLayout.formats.size();
	}

	inline const GPUMemoryUsage PrimitiveBuffer::getUsage() const {
		return info.usage;
	}

	inline const u32 PrimitiveBuffer::indices() const {
		return info.indexLayout.elements;
	}

	inline const u32 PrimitiveBuffer::vertices() const {
		return info.vertexLayout[0].elements;
	}

	inline const u32 PrimitiveBuffer::elements() const {
		return isIndexed() ? indices() : vertices();
	}

	inline const GPUFormat PrimitiveBuffer::getIndexFormat() const {
		return isIndexed() ? info.indexLayout.formats[0].format : GPUFormat::NONE;
	}

}