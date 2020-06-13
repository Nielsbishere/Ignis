#include "graphics/command/commands.hpp"

namespace ignis::cmd {

	//Flushing buffers

	void FlushBuffer::prepare(Graphics&, CommandList::Data *data) {

		//resize flushedRanges

		usz rangeCount = 1;

		if (PrimitiveBuffer *primitiveBuffer = pbuffer)
			rangeCount = primitiveBuffer->hasIndices() + primitiveBuffer->getVertexBuffers().size();

		if (flushedRanges.size() != rangeCount)
			flushedRanges.resize(rangeCount);

		//Prepare all affected buffers

		if (PrimitiveBuffer *primitiveBuffer = pbuffer) {

			if (GPUBuffer *buf = primitiveBuffer->getIndexBuffer().buffer)
				flushedRanges[0] = buf->prepare(data, uploadBuffer);

			u64 i = primitiveBuffer->hasIndices();

			for (auto &buf : primitiveBuffer->getVertexBuffers()) {
				flushedRanges[i] = buf.buffer->prepare(data, uploadBuffer);
				++i;
			}
		}

		else if (GPUBuffer *buffer = gbuffer)
			flushedRanges[0] = buffer->prepare(data, uploadBuffer);

	}

	void FlushBuffer::execute(Graphics&, CommandList::Data *data) const {

		//Perform all prepare all copies
		//	If a primitive buffer shares the GPUBuffer, it will automatically only flush the first time
		//	as this is how GPUBuffer is set-up.

		if (PrimitiveBuffer *primitiveBuffer = pbuffer) {

			if (GPUBuffer *buf = primitiveBuffer->getIndexBuffer().buffer)
				buf->flush(data, uploadBuffer, flushedRanges[0]);

			u64 i = primitiveBuffer->hasIndices();

			for (auto &buf : primitiveBuffer->getVertexBuffers()) {
				buf.buffer->flush(data, uploadBuffer, flushedRanges[i]);
				++i;
			}
		}

		else if (GPUBuffer *buffer = gbuffer)
			buffer->flush(data, uploadBuffer, flushedRanges[0]);

		else oic::System::log()->error("Flush buffer called without buffer");
	}

	//Flushing iages

	void FlushImage::prepare(Graphics&, CommandList::Data *data) {

		if(image)
			flushedRange = image->prepare(data, uploadBuffer);
	}

	void FlushImage::execute(Graphics&, CommandList::Data *data) const {

		if (!image) {
			oic::System::log()->error("Flush image called without texture");
			return;
		}

		image->flush(data, uploadBuffer, flushedRange);
	}

}