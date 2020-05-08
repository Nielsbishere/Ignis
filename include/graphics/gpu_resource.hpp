#pragma once
#include "graphics.hpp"

namespace ignis {

	struct RegisterLayout;
	struct GPUSubresource;

	//An object capable of being sent to a descriptor slot
	class GPUResource  {
	
	public:

		virtual bool isCompatible(
			const RegisterLayout &reg, const GPUSubresource &resource
		) const = 0;

	};
}