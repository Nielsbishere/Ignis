#include "system/system.hpp"
#include "system/log.hpp"
#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/enums.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"
#include "graphics/gl_context.hpp"
#include "graphics/memory/upload_buffer.hpp"
#include <cstring>

namespace ignis {

	GLenum glxBufferUsage(GPUMemoryUsage usage) {

		GLenum res{};

		bool cpuRead = HasFlags(usage, GPUMemoryUsage::CPU_READ);
		bool cpuWrite = HasFlags(usage, GPUMemoryUsage::CPU_WRITE);

		if (cpuRead || cpuWrite) {

			res |= GL_DYNAMIC_STORAGE_BIT;

			if (HasFlags(usage, GPUMemoryUsage::SHARED)) {

				res |= GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

				if (cpuRead)
					res |= GL_MAP_READ_BIT;

				if (cpuWrite)
					res |= GL_MAP_WRITE_BIT;

			}
		}

		if (HasFlags(usage, GPUMemoryUsage::SHARED))
			res |= GL_CLIENT_STORAGE_BIT;

		return res;
	}

	inline void fastCopy(volatile u8 *dst, const u8 *src, const u64 size) {

		u64 off = size & ~7;		//Bytes offset after the first 8 bytes

		if(size >> 3)
			std::copy((const u64*)src, (const u64*)(src + off), (volatile u64*)dst);

		//Copy next 4 bytes if available

		if (size & 4) {
			*(volatile u32*)(dst + off) = *(const u32*)(src + off);
			off += 4;
		}

		//Copy next 2 bytes if available

		if (size & 2) {
			*(volatile u16*)(dst + off) = *(const u16*)(src + off);
			off += 2;
		}

		//Copy next byte if available

		if (size & 1)
			*(volatile u8*)(dst + off) = *(const u8*)(src + off);
	}

	GPUBuffer::GPUBuffer(Graphics &g, const String &name, const Info &inf, GPUObjectType type):
		GPUObject(g, name, type), info(inf) {

		//Initialize buffer

		data = new Data();

		glCreateBuffers(1, &data->handle);
		GLuint handle = data->handle;

		glObjectLabel(GL_BUFFER, handle, GLsizei(name.size()), name.c_str());

		glNamedBufferStorage(
			handle, inf.size, {},
			glxBufferUsage(inf.usage)
		);

		GLenum mapFlags = GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;
		bool hasCpuAccess{};

		if (HasFlags(inf.usage, GPUMemoryUsage::CPU_WRITE)) {
			mapFlags |= GL_MAP_WRITE_BIT;
			hasCpuAccess = true;
		}

		if (HasFlags(inf.usage, GPUMemoryUsage::CPU_READ)) {
			mapFlags |= GL_MAP_READ_BIT;
			hasCpuAccess = true;
		}

		if (hasCpuAccess && HasFlags(inf.usage, GPUMemoryUsage::SHARED))
			data->unmapped = (volatile u8*)glMapNamedBufferRange(handle, 0, inf.size, mapFlags);
	}

	GPUBuffer::~GPUBuffer() {

		if (data->unmapped) {
			glUnmapNamedBuffer(data->handle);
			data->unmapped = nullptr;
		}

		glDeleteBuffers(1, &data->handle);
		delete data;
	}

	Pair<u64, u64> GPUBuffer::prepare(CommandList::Data*, UploadBuffer *uploadBuffer) {

		if (!HasFlags(info.usage, GPUMemoryUsage::CPU_WRITE)) {

			if (info.pending.empty() || info.markedPending)
				return { 0, u64_MAX };

			if (!uploadBuffer) {
				oic::System::log()->error("GPUBuffer::prepare without cpu access requires an upload buffer");
				return { 0, u64_MAX };
			}

			u64 size{};

			for (auto &pending : info.pending)
				size += pending.y;

			info.markedPending = true;

			return uploadBuffer->allocate(getGraphics().getData()->getContext().executionId, info.initData.data(), size, 1);
		}

		return { 0, u64_MAX };
	}

	void GPUBuffer::flush(CommandList::Data*, UploadBuffer *uploadBuffer, const Pair<u64, u64> &allocation){

		if (info.pending.empty())
			return;

		if (!HasFlags(info.usage, GPUMemoryUsage::CPU_WRITE)) {

			if (allocation.second == u64_MAX)
				return;

			if (info.initData.size() != info.size) {
				oic::System::log()->error("GPUBuffer doesn't have any backing CPU data");
				return;
			}

			auto &buffers = uploadBuffer->getInfo().buffers;
			auto it = buffers.find(allocation.first);

			if (it == buffers.end()) {
				oic::System::log()->error("GPUBuffer isn't found in the upload buffer");
				return;
			}

			GPUBuffer *buf = it->second;

			u64 offset{};

			for (auto &pending : info.pending) {

				u64 readOffset = allocation.second + offset;

				//Copy from upload buffer into this buffer

				glCopyNamedBufferSubData(buf->getData()->handle, data->handle, readOffset, pending.x, pending.y);

				offset += pending.y;
			}

			info.initData.clear();
		}

		else if (!HasFlags(info.usage, GPUMemoryUsage::SHARED) && !uploadBuffer) {
			oic::System::log()->error("Even though OpenGL handles UploadBuffers implictly, for non shared memory one is required by the ignis spec");
			return;
		}

		else if (data->unmapped) {

			for (auto &pending : info.pending) {

				const u8 *start = info.initData.data() + pending.x;
				volatile u8 *ptr = data->unmapped + pending.x;

				fastCopy(ptr, start, pending.y);

				glFlushMappedNamedBufferRange(data->handle, GLintptr(pending.x), GLsizeiptr(pending.y));
			}

		}

		else for (auto &pending : info.pending)
			glNamedBufferSubData(data->handle, GLintptr(pending.x), GLsizeiptr(pending.y), info.initData.data() + pending.x);

		info.pending.clear();
		info.markedPending = false;
	}

	Buffer GPUBuffer::readback(u64 offset, u64 size) {
		oicAssert("Can only readback from CPU visible memory", data->unmapped);
		oicAssert("Read out of bounds", offset + size <= info.size);
		return Buffer(data->unmapped + offset, data->unmapped + offset + size);
	}
}