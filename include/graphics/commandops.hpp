#pragma once
#include "types/types.hpp"

namespace ignis {

	//Command op codes

	//Techniques (FT or EXT)
	//Their availability can be queried from the Graphics class
	//When a FT or EXT is requested, it should return how available it is
	//	Supported: It is fully supported by hardware
	//	Performance: It runs slower but still works
	//	Unsupported: It won't be able to run
	//Suffixes:
	//	_0: Always available, either by hardware or software (performance impact when software)
	//  _1: Often available on desktop
	//  _2: Rarely available on desktop
	//  _3: Only available for certain vendors
	enum CommandOp : u32 {

		//Core commands

		CMD_BIND_PIPELINE,
		CMD_BIND_SHADER_REGISTERS,
		CMD_BIND_PRIMITIVE_BUFFERS,

		CMD_BEGIN_SURFACE,
		CMD_END_SURFACE,
		CMD_BEGIN_QUERY,
		CMD_END_QUERY,

		CMD_SET_CLEAR_COLOR,
		CMD_SET_VIEWPORT,
		CMD_SET_SCISSOR,
		CMD_SET_BLEND_CONSTANTS,
		CMD_SET_LINE_WIDTH,
		CMD_SET_STENCIL_COMPARE_MASK,
		CMD_SET_STENCIL_REFERENCE,
		CMD_SET_STENCIL_WRITE_MASK,

		CMD_DRAW_INSTANCED,
		CMD_DRAW_INDEXED_INSTANCED,

		CMD_UPDATE_REGISTER,
		CMD_UPDATE_CONSTANT,
		CMD_UPDATE_BUFFER,
		CMD_UPDATE_TEXTURE,

		CMD_CLEAR_SURFACE,
		CMD_CLEAR_IMAGE,
		CMD_CLEAR_QUERY_POOL,

		CMD_BLIT_IMAGE,
		CMD_COPY_BUFFER,
		CMD_COPY_IMAGE_TO_BUFFER,
		CMD_COPY_BUFFER_TO_IMAGE,
		CMD_COPY_QUERY_POOL,

		CMD_DEBUG_ADD_MARKER,
		CMD_DEBUG_START_MARKER,
		CMD_DEBUG_END_MARKER,

		//Software or hardware backed commands

		CMD_DRAW_INDIRECT_EXT_0,
		CMD_DRAW_INDEXED_INDIRECT_EXT_0,
		CMD_BEGIN_QUERY_INDIRECT_EXT_0,
		CMD_END_QUERY_INDIRECT_EXT_0,
		CMD_BEGIN_CONDITIONAL_EXT_0,
		CMD_END_CONDITIONAL_EXT_0,

		//API specific commands

		CMD_DISPATCH_FT_1,
		CMD_DISPATCH_INDIRECT_FT_EXT_1,

		//Common commands

		CMD_DRAW_INDIRECT_COUNT_EXT_2,
		CMD_DRAW_INDEXED_INDIRECT_COUNT_EXT_2,

		//Vendor commands

		CMD_TRACE_RAYS_FT_3,
		CMD_BUILD_ACCELERATION_STRUCTURE_FT_3,
		CMD_COPY_ACCELERATION_STRUCTURE_FT_3,
		CMD_WRITE_ACCELERATION_STRUCTURE_PROPERTIES_FT_3,
		CMD_DRAW_MESH_TASKS_FT_3,
		CMD_DRAW_MESH_TASKS_INDIRECT_FT_3,
		CMD_DRAW_MESH_TASKS_INDIRECT_COUNT_FT_3,
		//TODO: Variable rate shading

		//End of enum

		CMD_ENUM_NEXT
	};

}