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

		if (HasFlags(usage, GPUMemoryUsage::CPU_ACCESS)) {

			res |= GL_DYNAMIC_STORAGE_BIT;

			if(HasFlags(usage, GPUMemoryUsage::SHARED)) 
				res |= GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT;
		}

		if (HasFlags(usage, GPUMemoryUsage::SHARED))
			res |= GL_CLIENT_STORAGE_BIT;

		return res;
	}

	GPUBuffer::GPUBuffer(Graphics &g, const String &name, const Info &inf, GPUObjectType type):
		GPUObject(g, name, type), info(inf) {

		//Initialize buffer

		data = new Data();

		glCreateBuffers(1, &data->handle);
		GLuint handle = data->handle;

		glObjectLabel(GL_BUFFER, handle, GLsizei(name.size()), name.c_str());

		glNamedBufferStorage(
			handle, inf.size, nullptr,
			glxBufferUsage(inf.usage)
		);

		if (HasFlags(inf.usage, GPUMemoryUsage::CPU_ACCESS) && HasFlags(inf.usage, GPUMemoryUsage::SHARED))
			data->unmapped = (volatile u8*)glMapNamedBufferRange(handle, 0, inf.size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	}

	GPUBuffer::~GPUBuffer() {

		if (data->unmapped) {
			glUnmapNamedBuffer(data->handle);
			data->unmapped = nullptr;
		}

		glDeleteBuffers(1, &data->handle);
		delete data;
	}

	void GPUBuffer::flush(CommandList::Data*, UploadBuffer *uploadBuffer, u64 offset, u64 size) {

		if (offset + size > info.size) {
			oic::System::log()->error("GPUBuffer out of range");
			return;
		}

		if (!HasFlags(info.usage, GPUMemoryUsage::CPU_ACCESS)) {

			if (info.initData.size() != info.size) {
				oic::System::log()->error("GPUBuffer doesn't have any backing CPU data");
				return;
			}

			auto &ctx = getGraphics().getData()->getContext();

			auto allocation = uploadBuffer->allocate(ctx.executionId, getBuffer() + offset, size, 1);

			auto &buffers = uploadBuffer->getInfo().buffers;
			auto it = buffers.find(allocation.first);

			if (it == buffers.end()) {
				oic::System::log()->error("Invalid upload buffer allocation; GPUBuffer::flush failed");
				return;
			}

			glCopyNamedBufferSubData(it->second->getData()->handle, data->handle, allocation.second, offset, size);
			info.initData.clear();
			return;
		}

		if (!HasFlags(info.usage, GPUMemoryUsage::SHARED) && !uploadBuffer) {
			oic::System::log()->error("Even though OpenGL handles UploadBuffers implictly, for non shared memory one is required by the ignis spec");
			return;
		}

		if (data->unmapped) {

			const u8 *start = info.initData.data() + offset, *end = start + size;
			auto *ptr = data->unmapped + offset;

			switch (size & 7) {

				case 0:
					std::copy((const u64*)start, (const u64*)end, (volatile u64*)ptr);
					break;

				case 4:
					std::copy((const u32*)start, (const u32*)end, (volatile u32*)ptr);
					break;

				case 2:
				case 6:
					std::copy((const u16*)start, (const u16*)end, (volatile u16*)ptr);
					break;

				default:
					std::copy(start, end, ptr);
					break;

			}

			//TODO: Use explicit flushing and that should only be done once per frame

			//glFlushMappedNamedBufferRange(data->handle, GLintptr(offset), GLsizeiptr(size));
		}

		else glNamedBufferSubData(data->handle, GLintptr(offset), GLsizeiptr(size), info.initData.data() + offset);

	}
}