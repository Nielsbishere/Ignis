#pragma once
#include "graphics_object.hpp"

namespace ignis {

	struct RegisterLayout;

	//An object capable of being sent to a descriptor slot
	class GPUResource : public GraphicsObject {
	
	public:

		using GraphicsObject::GraphicsObject;

		virtual bool isCompatible(const RegisterLayout &reg) = 0;

	};
}