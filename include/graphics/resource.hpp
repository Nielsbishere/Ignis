#pragma once
#include "graphics_object.hpp"

namespace ignis {

	class ShaderRegister;

	//An object capable of being sent to a shader register
	class GPUResource : public GraphicsObject {
	
		friend class ShaderRegister;

	public:

		using GraphicsObject::GraphicsObject;

	protected:
	
		//TODO: Bind?

	};
}