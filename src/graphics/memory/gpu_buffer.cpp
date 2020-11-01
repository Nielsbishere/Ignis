#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/shader/pipeline_layout.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/enums.hpp"

namespace ignis {

	GPUBuffer::Info::Info(u64 bufferSize, GPUBufferUsage type, GPUMemoryUsage usage):
		initData(), size(bufferSize), type(type), usage(usage), pending { { 0, bufferSize }  }
	{
		if (!HasFlags(usage, GPUMemoryUsage::NO_CPU_MEMORY))
			initData.resize(bufferSize);
	}

	GPUBuffer::Info::Info(GPUBufferUsage type, GPUMemoryUsage usage, const Buffer &initData):
		initData(initData), size(initData.size()), type(type), usage(usage), pending { { 0, initData.size() } } {}

	bool GPUBuffer::isCompatible(GPUBufferType type, GPUBufferUsage usage) {

		switch (type) {

			case GPUBufferType::VERTEX:					return u32(usage) & u32(GPUBufferUsage::VERTEX);
			case GPUBufferType::INDEX:					return u32(usage) & u32(GPUBufferUsage::INDEX);
			case GPUBufferType::UNIFORM:				return u32(usage) & u32(GPUBufferUsage::UNIFORM);

			case GPUBufferType::STRUCTURED:
			case GPUBufferType::STORAGE:				return u32(usage) & u32(GPUBufferUsage::STORAGE);

			case GPUBufferType::INDIRECT_DRAW:
			case GPUBufferType::INDIRECT_DISPATCH:		return u32(usage) & u32(GPUBufferUsage::INDIRECT);

			case GPUBufferType::STAGING:				return u32(usage) & u32(GPUBufferUsage::STAGING);

			default:									return false;
		}
	}

	bool GPUBuffer::isCompatible(const RegisterLayout &reg, const GPUSubresource &resource) const {

		return reg.type == ResourceType::BUFFER && isCompatible(reg.bufferType, info.type) && 

			//Validate size
			(!reg.bufferSize || 
				(
					reg.bufferType != GPUBufferType::STRUCTURED && 
					reg.bufferSize == resource.bufferRange.size
				) ||
				(
					reg.bufferType == GPUBufferType::STRUCTURED && 
					(resource.bufferRange.size % reg.bufferSize) == 0
				)
			 ) &&

			//Validate write flags
			(!reg.isWritable	 || (u8(info.usage) & u8(GPUMemoryUsage::GPU_WRITE)));
	}

	void GPUBuffer::flush(u64 offset, u64 size) {

		if (offset + size > info.size)
			oic::System::log()->fatal("GPUBuffer::flush out of bounds");

		info.pending.push_back({ offset, size });
		mergePending();
	}

	void GPUBuffer::mergePending() {

		//Resolve all pending
		//For example if someone flushes range 0->16 and then flushes 12->16
		//	it should merge those into one

		constexpr isz delta = 127;	//If there's a difference of <=127 bytes, it will merge the range

		auto *back = &info.pending.back();

		do {

			auto *oldBack = back;

			for (auto &pending : info.pending) {

				if (&pending == back)
					continue;

				//Get signed distances between end and begin

				isz diffBegin = isz(pending.x + pending.y - back->x);
				isz diffEnd = isz(pending.x - back->x - back->y);

				if (diffBegin >= -delta && diffEnd <= delta) {

					auto start = pending.x;

					//Get the lowest and highest locations and replace the two allocations with one

					pending.x = std::min(start, back->x);
					pending.y = std::max(start + pending.y, back->x + back->y) - pending.x;

					usz off = &pending - info.pending.data();

					info.pending.erase(info.pending.begin() + (back - info.pending.data()));
					back = info.pending.data() + off;
					break;
				}
			}

			if (oldBack == back || info.pending.size() == 1)
				break;

		} while (true);

	}

}