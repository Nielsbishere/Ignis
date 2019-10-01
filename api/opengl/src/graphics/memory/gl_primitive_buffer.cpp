#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/gl_primitive_buffer.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"

namespace ignis {

	void PrimitiveBuffer::init() {

		data = new Data();

		glGenVertexArrays(1, &data->handle);
		glBindVertexArray(data->handle);
		
		for (auto &v : info.vertexLayout) {

			glBindBuffer(GL_ARRAY_BUFFER, v.buffer->getData()->handle);

			for (auto &elem : v.formats) {
				glEnableVertexAttribArray(elem.index);
				glVertexAttribPointer(
					elem.index, 
					GLint(FormatHelper::getChannelCount(elem.format)),
					glGpuFormat(elem.format),
					!FormatHelper::isUnnormalized(elem.format), 
					GLsizei(v.stride),
					(void*)usz(elem.offset)
				);

				if (v.isInstanced)
					glVertexAttribDivisor(elem.index, 1);
			}
		}

		if(isIndexed())
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info.indexLayout.buffer->getData()->handle);
	}

	void PrimitiveBuffer::destroy() {
		glDeleteVertexArrays(1, &data->handle);
		delete data;
	}
}