#pragma once
#include "../enums.hpp"
#include <cstring>

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

			inline bool operator==(const Attrib &other) const {
				return *(u64*)&offset == *(u64*)&other.offset && format == other.format;
			}
		};

		BufferAttributes() : values(), stride(), instanced() {}

		BufferAttributes(bool isInstanced, const List<Attrib> &values) : values(values), stride(), instanced(isInstanced) {

			for (const Attrib &a : values)
				if (a.offset + FormatHelper::getSizeBytes(a.format) >= stride)
					stride = u32(a.offset + FormatHelper::getSizeBytes(a.format));
		}

		BufferAttributes(bool isInstanced, const Attrib &v) : BufferAttributes(isInstanced, List<Attrib>{ v }) {}

		template<typename ...args>
		BufferAttributes(u32 startIndex, const args &...arg) : values(sizeof...(arg)), instanced(false) {
			u32 i{}, offset{};
			(fillAttrib(i, startIndex, offset, arg), ...);
			stride = offset;
		}

		template<typename ...args>
		BufferAttributes(u32 startIndex, bool isInstanced, const args &...arg) : values(sizeof...(arg)), instanced(isInstanced) {
			u32 i{}, offset{};
			(fillAttrib(i, startIndex, offset, arg), ...);
			stride = offset;
		}

		inline const Attrib &operator[](usz i) const { return values[i]; }
		inline u32 getStride() const { return stride; }
		inline usz size() const { return values.size(); }
		inline auto begin() { return values.begin(); }
		inline auto end() { return values.end(); }
		inline auto begin() const { return values.begin(); }
		inline auto end() const { return values.end(); }
		inline bool isInstanced() const { return instanced; }

		inline bool operator==(const BufferAttributes &other) const {
			return 
				values == other.values && stride == other.stride && 
				instanced == other.instanced;
		}

		inline bool operator!=(const BufferAttributes &other) const {
			return !operator==(other);
		}

	protected:

		inline void fillAttrib(u32 &i, u32 &index, u32 &offset, const GPUFormat &f) {
			values[i] = { offset, index, f };
			++i;
			++index;
			offset += u32(FormatHelper::getSizeBytes(f));
		}

	private:

		List<Attrib> values;
		u32 stride;
		bool instanced;
	};

	struct BufferLayout {

		Buffer initData{};
		BufferAttributes formats{};

		GPUBuffer *buffer{};
		u64 bufferOffset{};		//TODO: Require all buffer offsets to adhere to GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
		u32 elements{};

		template<typename T>
		BufferLayout(const List<T> &b, const BufferAttributes &formats, usz bufferOffset = 0) :
			initData((const u8*) b.data(), (const u8*) (b.data() + b.size())),
			formats(formats), bufferOffset(bufferOffset),
			elements(u32(initData.size() / formats.getStride())) {}

		BufferLayout(GPUBuffer *b, const BufferAttributes &formats, usz bufferOffset = 0);

		BufferLayout() {}

		inline u32 stride() const { return formats.getStride(); }
		inline usz size() const { return usz(elements) * stride(); }
		inline bool instanced() const { return formats.isInstanced(); }
		inline auto &operator[](usz i) const { return formats[i]; }
	};

}