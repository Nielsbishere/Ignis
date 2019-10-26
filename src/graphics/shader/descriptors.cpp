#include "graphics/shader/descriptors.hpp"
#include "graphics/gpu_resource.hpp"
#include "graphics/enums.hpp"

namespace ignis {

	Descriptors::Info::Info(const PipelineLayout &pipelineLayout, const HashMap<u32, GPUResource*> &resources):
		pipelineLayout(pipelineLayout), resources(resources) {

		for (auto &elem : pipelineLayout)
			if (resources.find(elem.first) == resources.end())
				this->resources[elem.first] = nullptr;
	}

	bool Descriptors::isResourceCompatible(u32 i, GPUResource *resource) const {

		auto it = info.resources.find(i);

		if (it == info.resources.end())
			return false;

		if (!resource)
			return true;

		auto regIt = info.pipelineLayout[i];

		if (regIt == info.pipelineLayout.end())
			return false;

		auto &reg = regIt->second;
		return resource->isCompatible(reg);
	}

	bool Descriptors::isShaderCompatible(const PipelineLayout &layout) const {
		return layout.supportsLayout(info.pipelineLayout);
	}
}