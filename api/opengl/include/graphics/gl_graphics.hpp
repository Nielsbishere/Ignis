#pragma once 
#include "types/types.hpp"
#include "graphics/graphics.hpp"

#ifdef _WIN32

	#define WIN32_LEAN_AND_MEAN

	#include <Windows.h>
	#include <GL/gl.h>
	#include "graphics/wglext.h"

	#undef ERROR
	#undef far
	#undef near
	#undef min
	#undef max
	#undef DOMAIN

	#include "glext.h"

#else

	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
	#include <GL/glx.h>

#endif

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

		GPUObjectId id;

		usz offset{}, size{};
		Vec3u16 dims{};		//For textures; since some can resize without changing id

		u32 subId{};
	};

	struct GLContext;

	struct Graphics::Data {

		//Per platform data

		plimpl struct Platform;

		Platform *platform{};

		//
		u64 executionId{};

		//OpenGL constants

		u8 maxSamples;
		f32 maxAnistropy;

		u32 major, minor;
		bool isES{};

		//Per context info

		HashMap<usz, GLContext*> contexts;
		HashMap<PrimitiveBuffer*, bool> primitiveBuffers;

		void updateContext(Graphics &g);
		void destroyContext();

		void storeContext(
			const List<GPUObject*> &resources, 
			void *callbackObjectPtr = nullptr, 
			void (*callbackPtr)(void*, UploadBuffer*, const Pair<u64, u64>&, TextureObject*, const Vec3u16&, const Vec3u16&, u16, u8, bool) = nullptr,
			TextureObject *gpuOutput = nullptr,
			UploadBuffer *cpuOutput = nullptr,
			const Pair<u64, u64> &allocation = {},
			Vec3u16 offset = {},
			Vec3u16 size = {},
			u16 layer = {},
			u8 mip = {},
			bool isStencil = {}
		);

		GLContext &getContext();

		//Helper functions

		static inline constexpr u64 getVersion(u32 major, u32 minor) {
			return (u64(major) << 32) | minor;
		}

		//Detect if it is the current version or higher
		inline bool version(u32 maj, u32 min) const {
			return getVersion(major, minor) >= getVersion(maj, min);
		}

	};

}

#include "gl_header.hpp"