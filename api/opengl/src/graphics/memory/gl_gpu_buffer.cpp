#include "system/system.hpp"
#include "system/log.hpp"
#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/enums.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"

namespace ignis {

	GPUBuffer::GPUBuffer(Graphics &g, const String &name, const Info &info):
		GraphicsObject(g, name), info(info), data(new Data()) {

		if(
			((
				info.type == GPUBufferType::STORAGE_FT || 
				info.type == GPUBufferType::STRUCTURED_FT
			) && !g.hasFeature(Feature::STORAGE_BUFFER)) ||
			(
				info.type == GPUBufferType::INDIRECT_DRAW_EXT && 
				!g.hasExtension(Extension::DRAW_INDIRECT)
			) ||
			(
				info.type == GPUBufferType::INDIRECT_DISPATCH_EXT && 
				!g.hasExtension(Extension::DISPATCH_INDIRECT)
			)
		)
			oic::System::log()->fatal("Couldn't create buffer, the type is not supported");

		//TODO: Texture buffers are also not standard...

		//Initialize buffer

		auto t = data->t = glBufferType(info.type);

		glGenBuffers(1, &data->handle);
		glBindBuffer(t, data->handle);

		glObjectLabel(GL_BUFFER, data->handle, GLsizei(getName().size()), getName().c_str());

		bool persistent = g.getData()->version(4, 4) && (u8(info.usage) & u8(GPUBufferUsage::SHARED));

		if (glBufferStorage)
			glBufferStorage(
				t, info.size, info.initData.data(),
				glBufferUsage(info.usage, persistent)
			);
		else
			glBufferData(
				t, info.size, info.initData.data(), 
				glBufferHint(info.usage)
			);

		if (u8(info.usage) & u8(GPUBufferUsage::CPU_WRITE)) {

			if (persistent)
				data->unmapped = (u8*)glMapBufferRange(t, 0, info.size, GL_MAP_WRITE_BIT);
			
		} else
			this->info.initData.clear();
	}

	GPUBuffer::~GPUBuffer() {

		if (data->unmapped) {
			glUnmapBuffer(data->handle);
			data->unmapped = nullptr;
		}

		glDeleteBuffers(1, &data->handle);
	}

	void GPUBuffer::flush(usz offset, usz size) {

		if (offset > info.size || offset + size > info.size) {
			oic::System::log()->fatal("GPUBuffer out of range");
			return;
		}

		if (u8(info.usage) & u8(GPUBufferUsage::CPU_WRITE)) {

			auto *gdat = getGraphics().getData();

			if (data->unmapped) {

				memcpy(data->unmapped, info.initData.data() + offset, size);
				gdat->bind(glBindBuffer, data->t, data->handle);
				glFlushMappedBufferRange(data->t, offset, size);

			} else {
				gdat->bind(glBindBuffer, data->t, data->handle);
				glBufferSubData(data->t, offset, size, info.initData.data() + offset);
			}

		} else
			oic::System::log()->fatal("GPUBuffer wasn't created with CPU write flags");
	}
}