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

			//TODO: Multi size arrays

			template<template<typename T2> typename T, typename T2, typename = std::enable_if_t<std::is_pod_v<T2>>>
			Layout(usz offset, const T<T2> &dat) :
				initData((u8*)dat.data(), (u8*)(dat.data() + dat.size())),
				array{ dat.size() }, offset(offset),
				stride(usz(std::ceil(f64(sizeof(T2)) / 16) * 16)),
				length(sizeof(T2)), format(GPUFormat(0)) { }

			template<typename T, typename = std::enable_if_t<std::is_pod_v<T>>>
			Layout(usz offset, T &dat, const List<usz> &array = {}) :
				initData((u8*)&dat, (u8*)(&dat + 1)), array(array),
				offset(offset), stride(usz(std::ceil(f64(sizeof(T)) / 16) * 16)),
				length(sizeof(T)), format(GPUFormat(0)) { }
		};

		struct Info {

			HashMap<String, Layout> layout;
			GPUBuffer::Info bufferInfo;

			Info(const HashMap<String, Layout> &layout, GPUBufferType type, GPUMemoryUsage usage);
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