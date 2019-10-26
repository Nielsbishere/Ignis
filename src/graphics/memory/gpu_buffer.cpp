#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/shader/pipeline_layout.hpp"
#include "graphics/enums.hpp"

namespace ignis {

	GPUBuffer::Info::Info(usz bufferSize, GPUBufferType type, GPUBufferUsage usage):
		type(type), usage(usage), size(bufferSize), initData(bufferSize) {}

	GPUBuffer::Info::Info(const Buffer &initData, GPUBufferType type, GPUBufferUsage usage):
		type(type), usage(usage), size(initData.size()), initData(initData) {}

	bool GPUBuffer::isCompatible(const RegisterLayout &reg) {
		return
			reg.type == ResourceType::BUFFER && 
			(!u8(reg.bufferType) || reg.bufferType == info.type) && 
			(!reg.bufferSize	 || reg.bufferSize == info.size) && 
			(!reg.isWritable	 || (u8(info.usage) & u8(GPUBufferUsage::GPU_WRITE)));
	}

}