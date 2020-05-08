#include "graphics/shader/descriptors.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace ignis {

	//Since descriptors are a Vulkan/DX12 concept, these don't maintain API data (flush doesn't do anything)
	//They are instead bound runtime

	Descriptors::Descriptors(Graphics &g, const String &name, const Info &info) :
		GPUObject(g, name, GPUObjectType::DESCRIPTORS), info(info) {}

	Descriptors::~Descriptors() {}

	void Descriptors::flush(usz, usz) { }

	void Descriptors::updateDescriptor(u32 i, const GPUSubresource &range) {

		if(!isResourceCompatible(i, range))
			oic::System::log()->fatal("Couldn't call setResource with incompatible resource");

		info.resources[i] = range;
	}

}