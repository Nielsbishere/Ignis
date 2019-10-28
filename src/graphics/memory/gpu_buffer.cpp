#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/shader/pipeline_layout.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/enums.hpp"

namespace ignis {

	GPUBuffer::Info::Info(usz bufferSize, GPUBufferType type, GPUMemoryUsage usage):
		type(type), usage(usage), size(bufferSize), initData(bufferSize) {}

	GPUBuffer::Info::Info(const Buffer &initData, GPUBufferType type, GPUMemoryUsage usage):
		type(type), usage(usage), size(initData.size()), initData(initData) {}

	bool GPUBuffer::isCompatible(const RegisterLayout &reg, const GPUSubresource &resource) {
		return
			reg.type == ResourceType::BUFFER && 
			(!u8(reg.bufferType) || reg.bufferType == info.type) && 
			(!reg.bufferSize	 || reg.bufferSize == resource.bufferRange.size) && //TODO: Structured buffer
			(!reg.isWritable	 || (u8(info.usage) & u8(GPUMemoryUsage::GPU_WRITE)));
	}

}