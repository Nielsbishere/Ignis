#pragma once
#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"

//General GPU commands
//These have to be implemented for every CommandList implementation

namespace ignis {

	class Pipeline;
	class Surface;
	class ShaderRegister;
	class PrimitiveBuffer;
	class Query;

	namespace cmd {

		//Basic commands

		template<CommandOp opCode>
		struct NoParamOp : public Command {
			NoParamOp(): Command(opCode, sizeof(*this)) {}
		};

		template<CommandOp opCode, typename BindObject>
		struct GraphicsObjOp : public Command {
			BindObject *bindObject;
			GraphicsObjOp(BindObject *bindObject, usz size = 0): 
				Command(opCode, size == 0 ? sizeof(*this) : size), 
				bindObject(bindObject) {}
		};

		using BindPipeline			= GraphicsObjOp<CMD_BIND_PIPELINE,			Pipeline>;
		using BindPrimitiveBuffer	= GraphicsObjOp<CMD_BIND_PRIMITIVE_BUFFER,  PrimitiveBuffer>;
		using BindShaderRegister	= GraphicsObjOp<CMD_BIND_SHADER_REGISTER,	ShaderRegister>;

		using BeginQuery			= GraphicsObjOp<CMD_BEGIN_SURFACE,			Query>;
		using EndQuery				= NoParamOp<CMD_END_QUERY>;
		using EndSurface			= NoParamOp<CMD_END_SURFACE>;
		using Present				= NoParamOp<CMD_PRESENT>;

		//Draw/dispatch commands

		struct DrawInstanced : public Command {

			u32 vertexStart, vertexCount, instanceStart, instanceCount;

			DrawInstanced(
				u32 vertexCount, u32 instanceCount = 1, u32 vertexStart = 0, u32 instanceStart = 0
			): 
				Command(CMD_DRAW_INSTANCED, sizeof(*this)),
				vertexStart(vertexStart), vertexCount(vertexCount), 
				instanceStart(instanceStart), instanceCount(instanceCount) {}
		};

		struct DrawIndexedInstanced : public Command {

			u32 indexStart, indexCount, vertexStart, instanceStart, instanceCount;

			DrawIndexedInstanced(
				u32 indexCount, u32 instanceCount = 1, u32 indexStart = 0,
				u32 instanceStart = 0, u32 vertexStart = 0
			): 
				Command(CMD_DRAW_INDEXED_INSTANCED, sizeof(*this)),
				indexStart(indexStart), indexCount(indexCount), vertexStart(vertexStart),
				instanceStart(instanceStart), instanceCount(instanceCount) {}
		};

		//Setting values

		template<CommandOp opCode, typename DataObject>
		struct DataOp : public Command {
			DataObject dataObject;
			DataOp(DataObject dataObject): Command(opCode, sizeof(*this)), dataObject(dataObject) {}
		};

		//using SetClearColor		= DataOp<CMD_SET_CLEAR_COLOR,		Vec4f
		using SetClearStencil		= DataOp<CMD_SET_CLEAR_STENCIL,			u32>;
		using SetClearDepth			= DataOp<CMD_SET_CLEAR_DEPTH,			f32>;
		using SetLineWidth			= DataOp<CMD_SET_LINE_WIDTH,			f32>;
		using SetBlendConstants		= DataOp<CMD_SET_BLEND_CONSTANTS,		Vec4f>;
		using SetStencilCompareMask = DataOp<CMD_SET_STENCIL_COMPARE_MASK,	u32>;
		using SetStencilWriteMask	= DataOp<CMD_SET_STENCIL_WRITE_MASK,	u32>;

		struct SetClearColor : public Command {

			enum Type : u8 {
				FLOAT, UNSIGNED_INT, SIGNED_INT
			} type;

			union {
				Vec4f rgbaf;
				Vec4u rgbau;
				Vec4i rgbai;
			};

			SetClearColor(const Vec4f &rgba): 
				Command(CMD_SET_CLEAR_COLOR, sizeof(*this)), rgbaf(rgba), type(FLOAT) {}

			SetClearColor(const Vec4u &rgba): 
				Command(CMD_SET_CLEAR_COLOR, sizeof(*this)), rgbau(rgba), type(UNSIGNED_INT) {}

			SetClearColor(const Vec4i &rgba): 
				Command(CMD_SET_CLEAR_COLOR, sizeof(*this)), rgbai(rgba), type(SIGNED_INT) {}
		};

		struct BeginSurface : GraphicsObjOp<CMD_BEGIN_SURFACE, Surface> {

			Vec4u renderArea;

			BeginSurface(Surface *surface, Vec2u renderSize = {}, Vec2u renderOffset = {}):
				GraphicsObjOp(surface, sizeof(*this)),
				renderArea{ renderOffset[0], renderOffset[1], renderSize[0], renderSize[1] } {}
		};
		
		//Debug calls

		template<CommandOp opCode, usz maxStringLength = 64>
		struct DebugOp : public Command {

			c8 string[maxStringLength];

			DebugOp(const String &str): Command(opCode, sizeof(*this)), string{} {
			
				if (str.size() > maxStringLength)
					oic::System::log()->fatal("Couldn't add debug operation; string is too big");

				memcpy(string, str.data(), str.size());
			}

			usz size() const {
				usz size = strlen(string);
				return size >= maxStringLength ? maxStringLength : size;
			}

		};

		using DebugStartRegion		= DebugOp<CMD_DEBUG_START_REGION>;
		using DebugInsertMarker		= DebugOp<CMD_DEBUG_INSERT_MARKER>;
		using DebugEndRegion		= NoParamOp<CMD_DEBUG_END_REGION>;

		//CMD_SET_STENCIL_REFERENCE,

		//CMD_UPDATE_REGISTER,
		//CMD_UPDATE_CONSTANT,
		//CMD_UPDATE_BUFFER,
		//CMD_UPDATE_TEXTURE,

		//CMD_CLEAR_SURFACE,
		//CMD_CLEAR_IMAGE,
		//CMD_CLEAR_QUERY_POOL,

		//CMD_BLIT_IMAGE,
		//CMD_COPY_BUFFER,
		//CMD_COPY_IMAGE_TO_BUFFER,
		//CMD_COPY_BUFFER_TO_IMAGE,
		//CMD_COPY_QUERY_POOL,

		////Software or hardware backed commands

		//CMD_DRAW_INDIRECT_EXT_0,
		//CMD_DRAW_INDEXED_INDIRECT_EXT_0,
		//CMD_BEGIN_QUERY_INDIRECT_EXT_0,
		//CMD_END_QUERY_INDIRECT_EXT_0,
		//CMD_BEGIN_CONDITIONAL_EXT_0,
		//CMD_END_CONDITIONAL_EXT_0,

		////API specific commands

		//CMD_DISPATCH_FT_1,
		//CMD_DISPATCH_INDIRECT_FT_EXT_1,

		////Common commands

		//CMD_DRAW_INDIRECT_COUNT_EXT_2,
		//CMD_DRAW_INDEXED_INDIRECT_COUNT_EXT_2,

		////Vendor commands

		//CMD_TRACE_RAYS_FT_3,
		//CMD_BUILD_ACCELERATION_STRUCTURE_FT_3,
		//CMD_COPY_ACCELERATION_STRUCTURE_FT_3,
		//CMD_WRITE_ACCELERATION_STRUCTURE_PROPERTIES_FT_3,
		//CMD_DRAW_MESH_TASKS_FT_3,
		//CMD_DRAW_MESH_TASKS_INDIRECT_FT_3,
		//CMD_DRAW_MESH_TASKS_INDIRECT_COUNT_FT_3,

	};
	

}