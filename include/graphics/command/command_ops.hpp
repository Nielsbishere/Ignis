#pragma once
#include "graphics/enums.hpp"

namespace ignis {

	//Used to determine how common an extension/feature is
	enum CommandOpSupport : u8 {
		CMD_SUPPORT_COMMON,				//_0
		CMD_SUPPORT_DESKTOP_COMMON,		//_1
		CMD_SUPPORT_DESKTOP_RARE,		//_2
		CMD_SUPPORT_VENDOR				//_3
	};

	//Command op codes

	//Techniques (FT or EXT)
	//	_FT: Feature (Requires a certain API)
	//	_EXT: Extension (Requires a certain GPU)
	//Their availability can be queried from the Graphics class
	//When a FT or EXT is requested, it should return how available it is
	//	Supported: It is fully supported by hardware
	//	Performance: It runs slower but still works
	//	Unsupported: It won't be able to run
	//Suffixes:
	//	none: Core standard
	//	_0: Rarely unavailable (>90%)
	//  _1: Often available on desktop (~50%)
	//  _2: Rarely available on desktop (<50%)
	//  _3: Only available for certain vendors (<25% generally)

	//>> 24 = technique
	//& 0x800000 = isFeature
	//& 0x600000 = support
	//& 0x1FFFFF = commandId (restarts per feature or extension)

	enum CommandOp : u32 {

		//Properties

		CMD_PROPERTY_TECHNIQUE_SHIFT	= 24,			//CommandOp >> N
		CMD_PROPERTY_TECHNIQUE			= 0xFF000000,
		CMD_PROPERTY_IS_FEATURE			= 0x00800000,
		CMD_PROPERTY_SUPPORT_SHIFT		= 21,			//Availability << N
		CMD_PROPERTY_SUPPORT			= 0x00600000,
		CMD_PROPERTY_ID					= 0x001FFFFF,

		//Core commands

		CMD_BIND_PIPELINE,
		CMD_BIND_DESCRIPTORS,
		CMD_BIND_PRIMITIVE_BUFFER,

		CMD_BEGIN_FRAMEBUFFER,
		CMD_END_FRAMEBUFFER,
		//CMD_BEGIN_QUERY,					//TODO:
		//CMD_END_QUERY,						//TODO:

		CMD_SET_CLEAR_COLOR,
		CMD_SET_CLEAR_DEPTH,
		CMD_SET_CLEAR_STENCIL,
		CMD_SET_SCISSOR,
		CMD_SET_VIEWPORT,
		CMD_SET_VIEWPORT_AND_SCISSOR,

		CMD_DRAW_INSTANCED,
		//CMD_DRAW_INDIRECT,					//TODO:
		CMD_DISPATCH,
		CMD_DISPATCH_INDIRECT,

		CMD_CLEAR_FRAMEBUFFER,
		CMD_CLEAR_IMAGE,
		CMD_CLEAR_BUFFER,
		//CMD_CLEAR_QUERY_POOL,				//TODO:

		//CMD_COPY_BUFFER,					//TODO:
		//CMD_COPY_IMAGE_TO_BUFFER,			//TODO:
		//CMD_COPY_BUFFER_TO_IMAGE,			//TODO:
		//CMD_COPY_QUERY_POOL,				//TODO:

		CMD_DEBUG_START_REGION,
		CMD_DEBUG_INSERT_MARKER,
		CMD_DEBUG_END_REGION,

		////Common extensions
		////TODO:

		//CMD_BEGIN_CONDITIONAL_EXT_0 = 
		//	(CMD_SUPPORT_COMMON << CMD_PROPERTY_SUPPORT_SHIFT) |
		//	(u32(Extension::CONDITIONAL_RENDERING) << CMD_PROPERTY_TECHNIQUE_SHIFT),

		//CMD_END_CONDITIONAL_EXT_0,

		////Common desktop extensions
		////TODO:

		//CMD_DRAW_INDIRECT_COUNT_EXT_1 = 
		//	(CMD_SUPPORT_DESKTOP_COMMON << CMD_PROPERTY_SUPPORT_SHIFT) |
		//	(u32(Extension::INDIRECT_COUNT) << CMD_PROPERTY_TECHNIQUE_SHIFT),

		//CMD_DRAW_INDEXED_INDIRECT_COUNT_EXT_1,

		////Vendor commands

		//CMD_TRACE_RAYS_FT_3 = 
		//	(CMD_SUPPORT_VENDOR << CMD_PROPERTY_SUPPORT_SHIFT) |
		//	(u32(Feature::RAYTRACING) << CMD_PROPERTY_TECHNIQUE_SHIFT),

		//CMD_BUILD_ACCELERATION_STRUCTURE_FT_3,
		//CMD_COPY_ACCELERATION_STRUCTURE_FT_3,
		//CMD_WRITE_ACCELERATION_STRUCTURE_PROPERTIES_FT_3,

		////TODO:

		//CMD_DRAW_MESH_TASKS_FT_3 =
		//	(CMD_SUPPORT_VENDOR << CMD_PROPERTY_SUPPORT_SHIFT) |
		//	(u32(Feature::MESH_SHADERS) << CMD_PROPERTY_TECHNIQUE_SHIFT),

		//CMD_DRAW_MESH_TASKS_INDIRECT_FT_3,
		//CMD_DRAW_MESH_TASKS_INDIRECT_COUNT_FT_3,
		////TODO: Variable rate shading
	};

}