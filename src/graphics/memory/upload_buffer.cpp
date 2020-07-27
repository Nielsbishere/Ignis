#include "graphics/memory/upload_buffer.hpp"

namespace ignis {

	UploadBuffer::UploadBuffer(Graphics &g, const String &name, const Info &inf):
		GPUObject(g, name, GPUObjectType::UPLOAD_BUFFER), info(inf)
	{
		//Create main buffer and allocation

		info.buffers[0] = new GPUBuffer(
			g, NAME(name + " buffer 0"), GPUBuffer::Info(
				inf.startSize, GPUBufferType::STAGING, GPUMemoryUsage::REQUIRE | GPUMemoryUsage::SHARED | GPUMemoryUsage::CPU_WRITE | GPUMemoryUsage::CPU_READ
			)
		);

		info.allocations.push_back({
			0,
			inf.startSize | Allocation::freeBit,
			0,
			0
		});
	}

	UploadBuffer::~UploadBuffer() {

		for (auto &buffer : info.buffers)
			buffer.second->loseRef();

		info.buffers.clear();
	}

	//Handle allocations

	Pair<u64, u64> UploadBuffer::allocate(u64 executionId, const u8 *data, const usz size, u32 alignment) {

		//Ensure we don't get thread interference 

		mutex.lock();

		//Find allocation

		auto &a = info.allocations;

		for (usz i{}, j = a.size(); i < j; ++i) {

			auto &ai = a[i];

			if (!ai.isFree()) continue;

			//Respect alignment

			u64 align = (alignment - ai.offset % alignment) % alignment;

			auto siz = ai.getSize();
			auto req = size + align;

			if (siz < req) continue;

			align += ai.offset;
			u64 bufferId = ai.bufferId;

			//Consume allocation

			if (siz == req) {
				ai.sizeAndFree = siz;
				ai.executionId = executionId;
			}

			//Generate two seperate allocations

			else {

				u64 leftOver = siz - req;

				ai.sizeAndFree = req;
				ai.executionId = executionId;

				a.insert(a.begin() + i + 1, Allocation{
					ai.offset + req,
					leftOver | Allocation::freeBit,
					u64_MAX,
					bufferId
				});
			}

			//Copy memory on cpu and shared memory

			auto *buf = info.buffers[bufferId];

			if (data)
				std::memcpy(buf->getBuffer() + align, data, size);
			else
				std::memset(buf->getBuffer() + align, 0, size);

			//Return allocation and finish mutex

			mutex.unlock();
			return { bufferId, align };
		}

		//Get the biggest allocation

		u64 biggest{};

		for (auto &elem : info.buffers)
			biggest = std::max(biggest, elem.second->size());
				
		//Create new allocation if possible

		if (biggest == info.maxSize) {
			oic::System::log()->error("UploadBuffer out of memory exception");
			mutex.unlock();
			return { 0, u64_MAX };
		}

		//Check if new allocation fits the resource

		u64 newSize = std::min(biggest * 2, info.maxSize);

		if (size > newSize && size < info.maxSize)
			newSize = std::min(size * 2, info.maxSize);

		if(size > newSize) {
			oic::System::log()->error("UploadBuffer out of memory exception");
			mutex.unlock();
			return { 0, u64_MAX };
		}

		//Create buffer

		u64 bufferId = ++info.bufferCounter;

		auto *buf = info.buffers[bufferId] = new GPUBuffer(
			getGraphics(), NAME(getName() + " buffer " + std::to_string(bufferId)),
			GPUBuffer::Info(
				newSize, GPUBufferType::STAGING, GPUMemoryUsage::REQUIRE | GPUMemoryUsage::SHARED | GPUMemoryUsage::CPU_READ | GPUMemoryUsage::CPU_WRITE
			)
		);

		//Create main allocation

		if (data)
			std::memcpy(buf->getBuffer(), data, size);
		else
			std::memset(buf->getBuffer(), 0, size);

		info.allocations.push_back(Allocation{
			0,
			size,
			executionId,
			bufferId
		});

		//Create left over block

		if(size != newSize)
			info.allocations.push_back(Allocation{
				size,
				(newSize - size) | Allocation::freeBit,
				u64_MAX,
				bufferId
			});

		//Return to program

		mutex.unlock();
		return { bufferId, 0 };
	}

	//Handle pushing to GPU

	void UploadBuffer::flush(CommandList::Data *cdata, u64 executionId) {

		mutex.lock();

		u64 prev = u64_MAX, bufferId = u64_MAX, biggest{};

		for(auto &alloc : info.allocations)
			if (alloc.executionId == executionId) {

				if (prev == u64_MAX) {
					prev = alloc.offset;
					biggest = prev + alloc.getSize();
					bufferId = alloc.bufferId;
				}
				else if (bufferId != alloc.bufferId) {

					auto it = info.buffers.find(bufferId);

					if (it != info.buffers.end())
						it->second->flush(cdata, nullptr, { prev, biggest });

					prev = u64_MAX;
				}
				else biggest = alloc.offset + alloc.getSize();

			} else if(prev != u64_MAX) {

				auto it = info.buffers.find(bufferId);

				if (it != info.buffers.end())
					it->second->flush(cdata, nullptr, { prev, biggest });

				prev = u64_MAX;
			}

		if (prev != u64_MAX) {

			auto it = info.buffers.find(bufferId);

			if (it != info.buffers.end())
				it->second->flush(cdata, nullptr, { prev, biggest });
		}

		mutex.unlock();
	}

	//Handle freeing memory

	void UploadBuffer::end(u64 executionId) {

		//Ensure we don't get thread interference 

		mutex.lock();

		//Constants

		auto &a = info.allocations;

		usz i{}, j = a.size();

		usz k = usz_MAX, bufferId{};

		for(; i < j; ++i) {

			auto &ai = a[i];

			bool isFree = ai.isFree();

			//Find first block of the freed frame
			if (k == usz_MAX) {

				if (!isFree && ai.executionId == executionId) {
					k = i;
					bufferId = ai.bufferId;
				}
			}

			//Check if we hit the end of our region
			else if (ai.executionId != executionId) {

				//Check if we have an empty block before we can merge with

				if (k > 0 && a[k - 1].isMergable(bufferId))
					--k;

				//Check if our current block can merge

				u64 end = ai.isMergable(bufferId) ? i + 1 : i;

				//Obtain initial offset, size and resource id

				u64 offset = a[k].offset;
				u64 size = (end >= j ? ai.offset + ai.getSize() : a[end].offset) - offset;

				//Remove from list & insert new block

				a.erase(a.begin() + k, a.begin() + end);

				//Decrement so the next check is at the right spot

				usz count = end - k;
				i -= count;
				j -= count;

				//Insert at the new position
				
				a.insert(a.begin() + i, Allocation{
					offset,
					size | Allocation::freeBit,
					u64_MAX,
					bufferId
				});

				++i;
				++j;

				//Reset our counter, so we search for next blocks

				k = usz_MAX;
			}
				
		}

		//If we filled up the last block, this will be ran

		if (k != usz_MAX) {

			//Possibly merge with left allocation

			if (k > 0 && a[k - 1].isMergable(bufferId))
				--k;

			//Obtain offset, size and id

			u64 offset = a[k].offset;
			u64 size = (a[j - 1].offset + a[j - 1].getSize()) - offset;

			//Erase blocks

			a.erase(a.begin() + k, a.begin() + j);

			//Insert ours

			a.push_back(Allocation{
				offset,
				size | Allocation::freeBit,
				u64_MAX,
				bufferId
			});

		}

		//Destroy last allocation if free, as long as there is a smaller allocation before it

		if (a[j - 1].isFree() && a[j - 1].getSize() > info.minSize) {

			auto &aj = a[j - 1];

			//Release buffer

			u64 oldSize = aj.getSize();
			auto buf = info.buffers.find(aj.bufferId);

			oicAssert("Invalid buffer referenced by allocation", buf == info.buffers.end());

			buf->second->loseRef();

			info.buffers.erase(aj.bufferId);
			a.erase(a.begin() + j - 1);

			//Insert smaller allocation if there's none left
			if (j == 1) {

				bufferId = ++info.bufferCounter;
				u64 newSize = std::max(info.minSize, oldSize / 2);

				info.buffers[bufferId] = new GPUBuffer(
					getGraphics(), NAME(getName() + " buffer " + std::to_string(bufferId)),
					GPUBuffer::Info(
						newSize, GPUBufferType::STAGING, GPUMemoryUsage::REQUIRE | GPUMemoryUsage::SHARED|		GPUMemoryUsage::CPU_WRITE | GPUMemoryUsage::CPU_READ
					)
				);

				info.allocations.push_back(Allocation{
					0,
					newSize | Allocation::freeBit,
					u64_MAX,
					bufferId
				});
			}

		}

		//Now others can access

		mutex.unlock();
	}

	//Copy memory

	Buffer UploadBuffer::readback(const Pair<u64, u64> &allocation, u64 size) {
		auto it = info.buffers.find(allocation.first);
		oicAssert("Buffer out of bounds", it != info.buffers.end());
		return it->second->readback(allocation.second, size);
	}

}