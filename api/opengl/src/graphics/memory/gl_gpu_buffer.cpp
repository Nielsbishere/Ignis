#include "system/system.hpp"
#include "system/log.hpp"
#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/enums.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"

namespace ignis {

	GPUBuffer::GPUBuffer(Graphics &g, const String &name, const Info &info):
		GraphicsObject(g, name), info(info) {

		//Initialize buffer

		data = new Data();

		glCreateBuffers(1, &data->handle);
		GLuint handle = data->handle;

		glObjectLabel(GL_BUFFER, handle, GLsizei(name.size()), name.c_str());

		bool persistent = u8(info.usage) & u8(GPUMemoryUsage::SHARED);

		glNamedBufferStorage(
			handle, info.size, info.initData.data(),
			glxBufferUsage(info.usage, persistent)
		);

		if (u8(info.usage) & u8(GPUMemoryUsage::CPU_WRITE)) {

			if (persistent)
				data->unmapped = 
					(u8*)glMapNamedBufferRange(handle, 0, info.size, GL_MAP_WRITE_BIT);
			
		} else
			this->info.initData.clear();
	}

	GPUBuffer::~GPUBuffer() {

		if (data->unmapped) {
			glUnmapNamedBuffer(data->handle);
			data->unmapped = nullptr;
		}

		glDeleteBuffers(1, &data->handle);
		delete data;
	}

	void GPUBuffer::flush(usz offset, usz size) {

		if (offset > info.size || offset + size > info.size) {
			oic::System::log()->fatal("GPUBuffer out of range");
			return;
		}

		if (u8(info.usage) & u8(GPUMemoryUsage::CPU_WRITE)) {

			if (data->unmapped) {

				memcpy(data->unmapped, info.initData.data() + offset, size);
				glFlushMappedNamedBufferRange(data->handle, offset, size);

			} else 
				glNamedBufferSubData(data->handle, offset, size, info.initData.data() + offset);

		} else
			oic::System::log()->fatal("GPUBuffer wasn't created with CPU write flags");
	}
}