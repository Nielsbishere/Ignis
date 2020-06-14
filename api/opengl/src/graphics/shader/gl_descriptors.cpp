#include "graphics/shader/descriptors.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace ignis {

	//Since descriptors are a Vulkan/DX12 concept, these don't maintain API data (flush doesn't do anything)
	//They are instead bound runtime

	Descriptors::Descriptors(Graphics &g, const String &name, const Info &inf) :
		GPUObject(g, name, GPUObjectType::DESCRIPTORS), info(inf)
	{
		info.flushedResources = inf.resources;
	}

	Descriptors::~Descriptors() {}

	//TODO: Make this a GPU concept like FlushBuffer and FlushImage
	//TODO: Keep refCount

	void Descriptors::flush(const List<Vec2u32> &ranges) {
	
		for (auto &range : ranges)

			if (range.x + range.y > info.resources.size())
				oic::System::log()->fatal("Descriptors::flush out of bounds");

			else for (auto i = range.x, j = i + range.y; i < j; ++i) {

				auto it = info.resources.find(i);

				if(it != info.resources.end())
					info.flushedResources[i] = it->second;
			}

	}

	void Descriptors::updateDescriptor(u32 i, const GPUSubresource &range) {

		if(!isResourceCompatible(i, range))
			oic::System::log()->fatal("Couldn't call setResource with incompatible resource");

		info.resources[i] = range;
	}

}