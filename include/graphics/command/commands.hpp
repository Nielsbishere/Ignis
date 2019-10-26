#pragma once
#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"

//General GPU commands
//These have to be implemented for every CommandList implementation

namespace ignis {

	class Pipeline;
	class Surface;
	class Descriptors;
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
		using BindDescriptors		= GraphicsObjOp<CMD_BIND_DESCRIPTORS,		Descriptors>;
		using BindPrimitiveBuffer	= GraphicsObjOp<CMD_BIND_PRIMITIVE_BUFFER,  PrimitiveBuffer>;

		using BeginQuery			= GraphicsObjOp<CMD_BEGIN_SURFACE,			Query>;
		using EndQuery				= NoParamOp<CMD_END_QUERY>;
		using EndSurface			= NoParamOp<CMD_END_SURFACE>;
		using Present				= NoParamOp<CMD_PRESENT>;

		//Draw/dispatch commands

		//Indexed or non-indexed; instanced draw call
		//DrawInstanced(...) for non-indexed
		//DrawInstanced::indexed() for indexed
		struct DrawInstanced : public Command {

			u32 start, count, instanceCount, instanceStart, vertexStart;
			bool isIndexed;

			DrawInstanced(
				u32 count, u32 instanceCount = 1, u32 start = 0, u32 instanceStart = 0
			): 
				Command(CMD_DRAW_INSTANCED, sizeof(*this)), isIndexed(false),
				start(start), count(count), instanceCount(instanceCount), vertexStart(), 
				instanceStart(instanceStart) {}

			static inline DrawInstanced indexed(u32 count, u32 instanceCount = 1, u32 start = 0, u32 instanceStart = 0, u32 vertexStart = 0) {
				DrawInstanced di(count, instanceCount, start, instanceStart);
				di.vertexStart = vertexStart;
				return di;
			}
		};

		//Setting values

		template<CommandOp opCode, typename DataObject>
		struct DataOp : public Command {
			DataObject dataObject;
			DataOp(DataObject dataObject): Command(opCode, sizeof(*this)), dataObject(dataObject) {}
		};

		using SetClearStencil		= DataOp<CMD_SET_CLEAR_STENCIL,			u32>;
		using SetClearDepth			= DataOp<CMD_SET_CLEAR_DEPTH,			f32>;
		using SetLineWidth			= DataOp<CMD_SET_LINE_WIDTH,			f32>;
		using SetBlendConstants		= DataOp<CMD_SET_BLEND_CONSTANTS,		Vec4f>;
		using SetStencilCompareMask = DataOp<CMD_SET_STENCIL_COMPARE_MASK,	u32>;
		using SetStencilWriteMask	= DataOp<CMD_SET_STENCIL_WRITE_MASK,	u32>;

		struct SetClearColor : public Command {

			enum class Type : u8 {
				FLOAT, UNSIGNED_INT, SIGNED_INT
			} type;

			union {
				Vec4f rgbaf;
				Vec4u rgbau;
				Vec4i rgbai;
			};

			SetClearColor(const Vec4f &rgba): 
				Command(CMD_SET_CLEAR_COLOR, sizeof(*this)), rgbaf(rgba), type(Type::FLOAT) {}

			SetClearColor(const Vec4u &rgba): 
				Command(CMD_SET_CLEAR_COLOR, sizeof(*this)), rgbau(rgba), type(Type::UNSIGNED_INT) {}

			SetClearColor(const Vec4i &rgba): 
				Command(CMD_SET_CLEAR_COLOR, sizeof(*this)), rgbai(rgba), type(Type::SIGNED_INT) {}
		};

		struct BeginSurface : GraphicsObjOp<CMD_BEGIN_SURFACE, Surface> {

			Vec4u renderArea;

			BeginSurface(Surface *surface, Vec2u renderSize = {}, Vec2u renderOffset = {}):
				GraphicsObjOp(surface, sizeof(*this)),
				renderArea{ renderOffset[0], renderOffset[1], renderSize[0], renderSize[1] } {}
		};

		struct BlitSurface : Command {

			Surface *src, *dst;
			Vec4u srcArea, dstArea;

			enum BlitMask : u8 {
				COLOR = 1, DEPTH = 2, STENCIL = 4,
				COLOR_DEPTH = 3, COLOR_STENCIL = 5,
				DEPTH_STENCIL = 6, ALL = 7
			} mask;

			enum BlitFilter : u8 {
				NEAREST, LINEAR
			} filter;

			BlitSurface(
				Surface *src, Surface *dst, Vec4u srcArea, Vec4u dstArea, BlitMask mask, BlitFilter filter
			): 
				Command(CMD_BLIT_SURFACE, sizeof(*this)), src(src), dst(dst), 
				srcArea(srcArea), dstArea(dstArea), mask(mask), filter(filter) {}

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

	};
	

}