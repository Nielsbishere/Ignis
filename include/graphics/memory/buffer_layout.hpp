#pragma once
#include "../enums.hpp"

namespace ignis {

	class GPUBuffer;

	class BufferAttributes {

	public:

		struct Attrib {

			u32 offset, index;
			GPUFormat format;

			Attrib(u32 offset, u32 index, GPUFormat format) :
				offset(offset), index(index), format(format) {}

			Attrib() : offset {}, index {}, format {} {}

			inline const bool operator==(const Attrib &other) const {
				return memcmp(this, &other, sizeof(*this)) == 0;
			}
		};

		BufferAttributes(bool isInstanced, const List<Attrib> &values) : values(values), instanced(isInstanced), stride() {

			for (const Attrib &a : values)
				if (a.offset + FormatHelper::getSizeBytes(a.format) >= stride)
					stride = u32(a.offset + FormatHelper::getSizeBytes(a.format));
		}

		BufferAttributes(bool isInstanced, const Attrib &v) : BufferAttributes(isInstanced, List<Attrib>{ v }) {}

		template<typename ...args>
		BufferAttributes(const args &...arg) : values(sizeof...(arg)), instanced(false) {
			u32 i {}, j {};
			(fillAttrib(i, j, arg), ...);
			stride = j;
		}

		template<typename ...args>
		BufferAttributes(bool isInstanced, const args &...arg) : values(sizeof...(arg)), instanced(isInstanced) {
			u32 i {}, j {};
			(fillAttrib(i, j, arg), ...);
			stride = j;
		}

		inline const Attrib &operator[](usz i) const { return values[i]; }
		inline const u32 getStride() const { return stride; }
		inline const usz size() const { return values.size(); }
		inline auto begin() { return values.begin(); }
		inline auto end() { return values.end(); }
		inline const bool isInstanced() const { return instanced; }

		inline const bool operator==(const BufferAttributes &other) const {
			return values == other.values && stride == other.stride && instanced == other.instanced;
		}

		inline const bool operator!=(const BufferAttributes &other) const {
			return !operator==(other);
		}

	protected:

		inline void fillAttrib(u32 &i, u32 &j, const GPUFormat &f) {
			values[i++] = { i, j, f };
			j += u32(FormatHelper::getSizeBytes(f));
		}

	private:

		List<Attrib> values;
		u32 stride;
		bool instanced;
	};

	struct BufferLayout {

		Buffer initData {};
		BufferAttributes formats;

		GPUBuffer *buffer {};
		u32 elements;

		template<typename T>
		BufferLayout(const List<T> &b, const BufferAttributes &formats) :
			initData((const u8*) b.data(), (const u8*) (b.data() + b.size())),
			formats(formats), elements(u32(initData.size() / formats.getStride())) {}

		BufferLayout(GPUBuffer *b, const BufferAttributes &formats);

		BufferLayout() : formats {}, elements {} {}

		inline const usz size() const { return usz(elements) * formats.getStride(); }
		inline const u32 stride() const { return formats.getStride(); }
		inline const bool instanced() const { return formats.isInstanced(); }
		inline const auto &operator[](usz i) const { return formats[i]; }
	};

}