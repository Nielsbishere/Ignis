#include "graphics/shader/descriptors.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace ignis {

	//Since descriptors are a Vulkan/DX12 concept, these don't maintain API data (flush doesn't do anything)
	//They are instead bound runtime

	Descriptors::Descriptors(Graphics &g, const String &name, const Info &info) :
		GraphicsObject(g, name), info(info) {}

	Descriptors::~Descriptors() {}

	void Descriptors::flush(usz, usz) { }

	void Descriptors::setResource(u32 i, GPUResource *resource) {

		if(!isResourceCompatible(i, resource))
			oic::System::log()->fatal("Couldn't call setResource with incompatible resource");

		info.resources[i] = resource;
	}

}