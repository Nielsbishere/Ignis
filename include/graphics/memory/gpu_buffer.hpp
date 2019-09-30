#pragma once
#include "graphics/graphics_object.hpp"

namespace ignis {

	enum class GPUBufferType : u8;
	enum class GPUBufferUsage : u8;

	class GPUBuffer : public GraphicsObject {

	public:

		struct Info {

			Buffer initData;
			usz size;

			GPUBufferType type;
			GPUBufferUsage usage;

			Info(usz bufferSize, GPUBufferType type, GPUBufferUsage usage):
				type(type), usage(usage), size(bufferSize), initData(bufferSize) {}

			Info(const Buffer &initData, GPUBufferType type, GPUBufferUsage usage):
				type(type), usage(usage), size(initData.size()), initData(initData) {}
		};

		apimpl struct Data;

		apimpl GPUBuffer(Graphics &g, const String &name, const Info &info);
		apimpl ~GPUBuffer();

		//Flush the updates from the CPU to the GPU
		apimpl void flush(usz offset, usz size);

		inline const usz size() const { return info.size; }

		Data *getData() { return data; }
		const Info &getInfo() const { return info; }

	private:

		Info info;
		Data *data;
	};

}