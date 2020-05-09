#pragma once
#include "graphics/gpu_resource.hpp"
#include "types/vec.hpp"

namespace ignis {

	enum class GPUBufferType : u8;
	enum class GPUMemoryUsage : u8;

	class GPUBuffer : public GPUObject, public GPUResource {

	public:

		struct Info {

			Buffer initData;
			usz size;

			GPUBufferType type;
			GPUMemoryUsage usage;

			Info(usz bufferSize, GPUBufferType type, GPUMemoryUsage usage);

			Info(const Buffer &initData, GPUBufferType type, GPUMemoryUsage usage);
		};

		apimpl struct Data;

		apimpl GPUBuffer(Graphics &g, const String &name, const Info &info);

		//Flush the updates from the CPU to the GPU
		apimpl void flush(const List<Vec2usz> &offsetsAndSizes);

		bool isCompatible(
			const RegisterLayout &reg, const GPUSubresource &resource
		) const final override;

		inline usz size() const { return info.size; }

		Data *getData() { return data; }
		const Info &getInfo() const { return info; }

		u8 *getBuffer() const { return (u8*) info.initData.data(); }

	protected:

		apimpl ~GPUBuffer();

	private:

		Info info;
		Data *data;
	};

}