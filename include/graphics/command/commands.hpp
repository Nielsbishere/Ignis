#pragma once
#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
#include "types/vec.hpp"

//General GPU commands
//These have to be implemented for every CommandList implementation

namespace ignis {

	class Pipeline;
	class Framebuffer;
	class Descriptors;
	class PrimitiveBuffer;
	class Query;
	class GPUBuffer;
	class Texture;

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

		/*using BeginQuery			= GraphicsObjOp<CMD_BEGIN_QUERY,			Query>;
		using EndQuery				= NoParamOp<CMD_END_QUERY>;*/
		using EndFramebuffer		= NoParamOp<CMD_END_FRAMEBUFFER>;

		struct BeginFramebuffer : public Command {

			Framebuffer *target;

			BeginFramebuffer(Framebuffer *target):
				Command(CMD_BEGIN_FRAMEBUFFER, sizeof(*this)), target(target) {}

		};

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
				Command(CMD_DRAW_INSTANCED, sizeof(*this)),
				start(start), count(count), instanceCount(instanceCount), 
				instanceStart(instanceStart), vertexStart(), isIndexed(false) {}

			static inline DrawInstanced indexed(u32 count, u32 instanceCount = 1, u32 start = 0, u32 instanceStart = 0, u32 vertexStart = 0) {
				DrawInstanced di(count, instanceCount, start, instanceStart);
				di.vertexStart = vertexStart;
				di.isIndexed = true;
				return di;
			}
		};

		//Dispatch calls

		struct Dispatch : public Command {

			Vec3u32 threadCount;

			Dispatch(u32 threads) :
				Command(CMD_DISPATCH, sizeof(*this)),
				threadCount { threads, 1, 1 } {}

			Dispatch(const Vec2u32 &threads) :
				Command(CMD_DISPATCH, sizeof(*this)),
				threadCount { threads.x, threads.y, 1 } {}

			Dispatch(const Vec3u32 &threads) :
				Command(CMD_DISPATCH, sizeof(*this)),
				threadCount(threads) {}

		};

		struct DispatchIndirect : public Command {

			GPUBuffer *buffer;
			usz offset;			//in dispatch indirect instructions

			DispatchIndirect(GPUBuffer *buffer, usz offset = 0) :
				Command(CMD_DISPATCH_INDIRECT, sizeof(*this)),
				buffer(buffer),
				offset(offset) {}

		};

		//Setting values

		template<CommandOp opCode, typename DataObject>
		struct DataOp : public Command {
			DataObject dataObject;
			DataOp(DataObject dataObject): Command(opCode, sizeof(*this)), dataObject(dataObject) {}
		};

		using SetClearStencil		= DataOp<CMD_SET_CLEAR_STENCIL,			u8>;
		using SetClearDepth			= DataOp<CMD_SET_CLEAR_DEPTH,			f32>;

		struct SetClearColor : public Command {

			union {
				Vec4f32 rgbaf;
				Vec4u32 rgbau;
				Vec4i32 rgbai;
			};

			enum class Type : u8 {
				FLOAT, UNSIGNED_INT, SIGNED_INT
			} type;

			SetClearColor() : SetClearColor(Vec4f32()) {}

			SetClearColor(const Vec4f32 &rgba): 
				Command(CMD_SET_CLEAR_COLOR, sizeof(*this)), rgbaf(rgba), type(Type::FLOAT) {}

			SetClearColor(const Vec4u32 &rgba): 
				Command(CMD_SET_CLEAR_COLOR, sizeof(*this)), rgbau(rgba), type(Type::UNSIGNED_INT) {}

			SetClearColor(const Vec4i32 &rgba): 
				Command(CMD_SET_CLEAR_COLOR, sizeof(*this)), rgbai(rgba), type(Type::SIGNED_INT) {}
		};

		template<CommandOp opCode>
		struct SetViewRegion : public Command {

			Vec2i32 offset;
			Vec2u32 size;

			SetViewRegion(const Vec2u32 &size = {}, const Vec2i32 &offset = {}) :
				Command(opCode, sizeof(*this)),
				offset(offset), size(size) {}
		};

		using SetScissor = SetViewRegion<CMD_SET_SCISSOR>;
		using SetViewport = SetViewRegion<CMD_SET_VIEWPORT>;
		using SetViewportAndScissor = SetViewRegion<CMD_SET_VIEWPORT_AND_SCISSOR>;

		//Copy commands

		struct ClearFramebuffer : Command {

			Framebuffer *target;

			enum ClearFlags : u8 {
				COLOR = 1,
				DEPTH = 2,
				STENCIL = 4,
				DEPTH_STENCIL = 6,
				ALL = 7
			} clearFlags;

			ClearFramebuffer(Framebuffer *target, ClearFlags clearFlags = ClearFlags::ALL) :
				Command(CMD_CLEAR_FRAMEBUFFER, sizeof(*this)), target(target), clearFlags(clearFlags) {}
		};

		struct ClearImage : Command {

			Texture *texture;
			Vec2i16 offset;
			Vec2u16 size;
			u16 mipLevel;
			u16 minSlice, maxSlice;

			ClearImage(
				Texture *texture,
				u16 mipLevel = {}, u16 minSlice = {}, u16 maxSlice = {},
				const Vec2u16 &size = {}, const Vec2i16 &offset = {}
			) :
				Command(CMD_CLEAR_IMAGE, sizeof(*this)), 
				texture(texture), mipLevel(mipLevel), minSlice(minSlice),
				offset(offset), size(size), maxSlice(maxSlice) {}

		};

		struct ClearBuffer : Command {

			GPUBuffer *buffer;
			usz offset, size;

			ClearBuffer(GPUBuffer *buffer, usz offset = 0, usz size = 0) :
				Command(CMD_CLEAR_BUFFER, sizeof(*this)),
				buffer(buffer), offset(offset), size(size) {}

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