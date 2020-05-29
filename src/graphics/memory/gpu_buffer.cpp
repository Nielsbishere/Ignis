#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/shader/pipeline_layout.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/enums.hpp"

namespace ignis {

	GPUBuffer::Info::Info(u64 bufferSize, GPUBufferType type, GPUMemoryUsage usage):
		initData(bufferSize), size(bufferSize), type(type), usage(usage) {}

	GPUBuffer::Info::Info(const Buffer &initData, GPUBufferType type, GPUMemoryUsage usage):
		initData(initData), size(initData.size()), type(type), usage(usage) {}

	bool GPUBuffer::isCompatible(
		const RegisterLayout &reg, const GPUSubresource &resource
	) const {
		return
			reg.type == ResourceType::BUFFER && 
			(!u8(reg.bufferType) || reg.bufferType == info.type) && 

			//Validate size
			(!reg.bufferSize	 || 
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

}