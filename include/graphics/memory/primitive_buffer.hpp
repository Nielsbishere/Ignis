#pragma once
#include "../graphics.hpp"
#include "buffer_layout.hpp"

namespace ignis {

	class PrimitiveBuffer : public GPUObject {

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

		inline const BufferLayout &getVertexBuffer(usz i) const { return info.vertexLayout[i]; }
		inline const BufferLayout &getIndexBuffer() const { return info.indexLayout; }
		inline const List<BufferLayout> &getVertexBuffers() const { return info.vertexLayout; }

		inline bool hasIndices() const { return indices(); }

		inline GPUMemoryUsage getUsage() const { return info.usage; }
		inline GPUFormat getIndexFormat() const { return hasIndices() ? info.indexLayout.formats[0].format : GPUFormat::NONE; }

		inline u32 indices() const { return info.indexLayout.elements; }
		inline u32 vertices() const { return info.vertexLayout[0].elements; }
		inline u32 elements() const { return hasIndices() ? indices() : vertices(); }
		inline u32 vertexBuffers() const { return u32(info.vertexLayout.size()); }

		inline Data *getData() { return data; }
		inline const Info &getInfo() const { return info; }

		//Whether or not this primitive buffer matches a layout
		bool matchLayout(const List<BufferAttributes> &layout) const;

	private:

		~PrimitiveBuffer();

		Info info;
		Data *data{};
	};

	using PrimitiveBufferRef = GraphicsObjectRef<PrimitiveBuffer>;
}