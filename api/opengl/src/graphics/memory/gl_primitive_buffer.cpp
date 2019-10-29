#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/gl_primitive_buffer.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"

namespace ignis {

	void PrimitiveBuffer::init() {

		data = new Data();

		glCreateVertexArrays(1, &data->handle);
		glObjectLabel(
			GL_VERTEX_ARRAY, data->handle, GLsizei(getName().size()), getName().c_str()
		);

		GLuint handle = data->handle;

		u32 i{};
		
		for (auto &v : info.vertexLayout) {

			glVertexArrayVertexBuffer(
				handle, i, v.buffer->getData()->handle, v.bufferOffset, v.stride()
			);

			for (auto &elem : v.formats) {

				glEnableVertexArrayAttrib(handle, elem.index);
				glVertexArrayAttribFormat(
					handle,
					elem.index, 
					GLint(FormatHelper::getChannelCount(elem.format)),
					glxGpuFormatType(elem.format),
					!FormatHelper::isUnnormalized(elem.format), 
					elem.offset
				);

				glVertexArrayAttribBinding(handle, elem.index, i);

				if (v.instanced())
					glVertexArrayBindingDivisor(handle, elem.index, 1);
			}
		}

		if(isIndexed())
			glVertexArrayElementBuffer(
				handle, info.indexLayout.buffer->getData()->handle
			);
	}

	void PrimitiveBuffer::destroy() {
		glDeleteVertexArrays(1, &data->handle);
		delete data;
	}
}