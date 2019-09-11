#pragma once 
#include "graphics/graphics.hpp"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <gl/GL.h>
#endif

#include "glext.h"

namespace ignis {

	class Surface;

	struct Graphics::Data {

		Surface *currentSurface;

		f32 depth{};
		u32 stencil{};

		Vec4u xywh{};

		Vec4f clearColor{};

		u32 maxSamples;

		u32 minor, major;
		bool isES {};

	};

}