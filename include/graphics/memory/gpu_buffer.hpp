#pragma once
#include "graphics/gpu_resource.hpp"
#include "graphics/command/command_list.hpp"
#include "types/vec.hpp"

namespace ignis {

	enum class GPUBufferType : u8;
	enum class GPUMemoryUsage : u8;

	class GPUBuffer : public GPUObject, public GPUResource {

		friend class CommandList;
		friend class UploadBuffer;

	public:

		struct Info {

			Buffer initData;
			u64 size;

			GPUBufferType type;
			GPUMemoryUsage usage;

			Info(u64 bufferSize, GPUBufferType type, GPUMemoryUsage usage);

			Info(const Buffer &initData, GPUBufferType type, GPUMemoryUsage usage);
		};

		apimpl struct Data;

		GPUBuffer(Graphics &g, const String &name, const Info &info):
			GPUBuffer(g, name, info, GPUObjectType::BUFFER) {}

		bool isCompatible(
			const RegisterLayout &reg, const GPUSubresource &resource
		) const final override;

		inline u64 size() const { return info.size; }

		Data *getData() { return data; }
		const Info &getInfo() const { return info; }

		u8 *getBuffer() const { return (u8*) info.initData.data(); }

	protected:

		apimpl GPUBuffer(Graphics &g, const String &name, const Info &info, GPUObjectType type);

		apimpl void flush(CommandList::Data *data, UploadBuffer *uploadBuffer, u64 offset, u64 size);

		apimpl ~GPUBuffer();

	private:

		Info info;
		Data *data;
	};

}