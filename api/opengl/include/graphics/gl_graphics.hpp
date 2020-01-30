#pragma once 
#include "types/types.hpp"
#include "graphics/graphics.hpp"

#ifdef _WIN32

	#define WIN32_LEAN_AND_MEAN

	#include <Windows.h>
	#include <gl/GL.h>
	#include "graphics/wglext.h"

	#undef ERROR
	#undef far
	#undef near
	#undef min
	#undef max
	#undef DOMAIN

#endif

#include "glext.h"

namespace ignis {

	class Framebuffer;
	class PrimitiveBuffer;
	class Pipeline;
	class Descriptors;

	enum class CullMode : u8;
	enum class WindMode : u8;
	enum class FillMode : u8;

	//Graphics data

	struct BoundRange {
		GLuint handle{};
		usz offset{}, size{};
	};

	struct GLContext;

	struct Graphics::Data {

		//Per platform data

		plimpl struct Platform;

		Platform *platform{};

		//OpenGL constants

		u8 maxSamples;
		f32 maxAnistropy;

		u32 major, minor;
		bool isES{};

		//Per context info

		HashMap<usz, GLContext> contexts;
		HashMap<PrimitiveBuffer*, bool> primitiveBuffers;

		void updateContext();
		void destroyContext();
		GLContext &getContext();

		//Helper functions

		static inline constexpr u64 getVersion(u32 major, u32 minor) {
			return (u64(major) << 32) | minor;
		}

		//Detect if it is the current version or higher
		inline bool version(u32 maj, u32 min) const {
			return getVersion(major, minor) >= getVersion(maj, min);
		}

		void removeTexture(GLuint tex);
	};

}

#include "gl_header.hpp"