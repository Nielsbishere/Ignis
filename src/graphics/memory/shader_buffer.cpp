#include "graphics/memory/shader_buffer.hpp"
#include <cstring>

namespace ignis {

	static inline usz bufferSize(const HashMap<String, ShaderBuffer::Layout> &layout) {

		usz length{};

		for (auto &l : layout) {

			usz j = l.second.array.size() ? l.second.stride : 0;

			for (const usz &a : l.second.array)
				j *= a;

			usz ou = j + l.second.length;
			
			if (ou > length)
				length = ou;
		}

		return length;
	}

	ShaderBuffer::Info::Info(GPUBufferType type, GPUMemoryUsage usage, const HashMap<String, Layout> &layout) :
		bufferInfo(bufferSize(layout), type, usage) {

		for (auto &l : layout) {

			if (l.second.initData.empty())
				continue;

			usz j = l.second.array.size() ? l.second.array[0] : 1;

			for (usz i = 0; i < j; ++i)
				std::memcpy(
					bufferInfo.initData.data() + l.second.offset + l.second.stride * i,
					l.second.initData.data() + l.second.length * i,
					l.second.length
				);
		}

	}

	ShaderBuffer::ShaderBuffer(Graphics &g, const String &name, const ShaderBuffer::Info &info):
		GPUBuffer(g, name, info.bufferInfo), info(info) { }

}