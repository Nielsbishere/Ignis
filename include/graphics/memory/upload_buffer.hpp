#pragma once
#include "gpu_buffer.hpp"
#include <mutex>

namespace ignis {

	//A class for pushing data to the GPU via CPU driver memory
	//It can automatically shrink & grow and handle multi threaded allocations
	class UploadBuffer : public GPUObject {

		friend class CommandList;
		friend class Graphics;

	public:

		struct Allocation {

			static constexpr u64 freeBit = 1_u64 << 63;

			u64 offset{}, sizeAndFree{}, executionId{}, bufferId{};

			inline bool isFree() const { return sizeAndFree >> 63; }
			inline u64 getSize() const { return sizeAndFree << 1 >> 1; }

			inline bool isMergable(u64 bid) const { return isFree() && bufferId == bid; }
		};

		struct Info {

			u64 minSize, maxSize, startSize;

			u64 bufferCounter{};

			List<Allocation> allocations;		//Allocations for these resources
			HashMap<u64, GPUBuffer*> buffers;	//All resources currently allocated

			Info(u64 size, u64 minSize, u64 maxSize):
				minSize(minSize ? minSize : size), startSize(size),
				maxSize(maxSize ? maxSize : size) {}
		};

		UploadBuffer(Graphics &g, const String &name, const Info &info);
		
		//Place an allocation into this buffer; returns false if nothing is available
		//executionId is the id of the current execution being made available
		//it requires executionId to be the same until end is called
		//Returns (bufferId, allocation)
		//Returns { 0, u64_MAX } if no allocation possible
		Pair<u64, u64> allocate(u64 executionId, const u8 *data, const usz size, u32 alignment);

		//Readback result
		Buffer readback(const Pair<u64, u64> &allocation, u64 size);

		const Info &getInfo() const { return info; }

	protected:

		//Call when an execution has ended again, so the memory becomes available
		void end(u64 executionId);

		//Flush all allocations from the execution
		void flush(CommandList::Data*, u64 executionId);

	private:

		~UploadBuffer();

		std::mutex mutex;
		Info info;
	};

	using UploadBufferRef = GraphicsObjectRef<UploadBuffer>;
}