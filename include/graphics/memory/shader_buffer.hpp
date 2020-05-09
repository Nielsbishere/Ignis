#pragma once
#include "gpu_buffer.hpp"

namespace ignis {

	enum class GPUFormat : u16;

	class ShaderBuffer : public GPUBuffer {

	public:

		struct Layout {

			List<u8> initData;
			List<usz> array;
			usz offset, stride, length;

			GPUFormat format;

			Layout(usz offset, const Buffer &data, usz stride = 0, const List<usz> &array = {}) :
				initData(data), array(array),
				offset(offset), stride(stride ? stride : data.size()),
				length(data.size()), format(GPUFormat(0)) { }

			Layout(usz offset, const usz size, usz stride = 0, const List<usz> &array = {}) :
				initData(), array(array),
				offset(offset), stride(stride ? stride : size),
				length(size), format(GPUFormat(0)) { }
		};

		struct Info {

			HashMap<String, Layout> layout;
			GPUBuffer::Info bufferInfo;

			Info(GPUBufferType type, GPUMemoryUsage usage, const HashMap<String, Layout> &layout);
		};

		ShaderBuffer(Graphics &g, const String &name, const ShaderBuffer::Info &info);

		/*template<typename T>
		void set(const String &name, const T &t);

		void flush();*/

		const Info &getInfo() const { return info; }

	private:

		~ShaderBuffer() = default;

		Info info;
	};

}