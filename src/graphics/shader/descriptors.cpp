#include "graphics/shader/descriptors.hpp"
#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/enums.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {

	Descriptors::Info::Info(const PipelineLayout &pipelineLayout, const Resources &resources):
		pipelineLayout(pipelineLayout), resources(resources) {

		for (auto &elem : pipelineLayout)
			if (resources.find(elem.first) == resources.end())
				this->resources[elem.first] = nullptr;
	}

	bool Descriptors::isResourceCompatible(u32 i, const GPUSubresource &resource) const {

		auto it = info.resources.find(i);

		if (it == info.resources.end())
			return false;

		if (!resource.resource)
			return true;

		auto regIt = info.pipelineLayout[i];

		if (regIt == info.pipelineLayout.end())
			return false;

		auto &reg = regIt->second;
		return resource.resource->isCompatible(reg, resource);
	}

	bool Descriptors::isShaderCompatible(const PipelineLayout &layout) const {
		return layout.supportsLayout(info.pipelineLayout);
	}

	GPUSubresource::GPUSubresource(GPUBuffer *resource, usz offset, usz size):
		resource(resource), bufferRange{ offset, size } {

		if (!resource || offset >= resource->size() || offset + size >= resource->size())
			oic::System::log()->fatal("Resource out of bounds");

		if (size == 0)
			bufferRange.size = resource->size() - offset;
	}

}