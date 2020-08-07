#include "graphics/shader/descriptors.hpp"
#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/memory/texture.hpp"
#include "graphics/shader/sampler.hpp"
#include "graphics/enums.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {

	Descriptors::Info::Info(
		const PipelineLayout *pipelineLayout, u16 descriptorIndex, 
		const Subresources &resources
	):
		pipelineLayout(pipelineLayout), resources(resources), 
		descriptorSetIndex(descriptorIndex)
	{

		if(pipelineLayout)
			for (auto &elem : pipelineLayout->getInfo())
				if (descriptorSetIndex == elem.second.descriptorSetId && resources.find(elem.first) == resources.end())
					this->resources[elem.first] = {};
	}

	bool Descriptors::isResourceCompatible(u32 i, const GPUSubresource &resource) const {

		auto it = info.resources.find(i);

		if (it == info.resources.end())
			return false;

		if (!resource.resource)
			return true;

		if (!info.pipelineLayout)
			return false;

		auto regIt = info.pipelineLayout->getInfo()[i];

		if (regIt == info.pipelineLayout->getInfo().end())
			return false;

		auto &reg = regIt->second;
		return resource.resource->isCompatible(reg, resource);
	}

	GPUSubresource::GPUSubresource(GPUBuffer *resource, usz offset, usz size):
		resource(resource), bufferRange{ offset, size } {

		if (!resource || offset >= resource->size() || offset + size >= resource->size())
			oic::System::log()->fatal("Resource out of bounds");

		if (size == 0)
			bufferRange.size = resource->size() - offset;
	}

	GPUSubresource::GPUSubresource(
		Sampler *sampler, TextureObject *texture, TextureType subType,
		u32 levelCount, u32 layerCount,
		u32 minLevel, u32 minLayer
	) :
		resource(sampler), 
		samplerData(
			texture, minLevel, minLayer, levelCount, layerCount,
			subType == TextureType::ENUM_END ? texture->getInfo().textureType : subType
		)
	{
		if (!levelCount) samplerData.levelCount = samplerData.texture->getInfo().mips;
		if (!layerCount) samplerData.layerCount = samplerData.texture->getInfo().layers;
	}

	GPUSubresource::GPUSubresource(
		TextureObject *resource,  TextureType subType,
		u32 levelCount, u32 layerCount,
		u32 minLevel, u32 minLayer
	): 
		resource(resource),
		textureRange(minLevel, minLayer, levelCount, layerCount, subType) {

		if (!levelCount) samplerData.levelCount = resource->getInfo().mips;
		if (!layerCount) samplerData.layerCount = resource->getInfo().layers;
	}

	GPUSubresource::GPUSubresource(Sampler *resource): 
		resource(resource), samplerData(){}


}