#pragma once
#include "../enums.hpp"
#include "../graphics_object.hpp"

namespace ignis {

	class GPUBuffer;

	struct BufferLayout {

		struct Attrib {

			u32 offset, index;
			GPUFormat format;

			Attrib(u32 offset, u32 index, GPUFormat format) :
				offset(offset), index(index), format(format) {}

			Attrib(): offset{}, index{}, format{} {}
		};

	private:

		inline void registerAttrib(u32 &i, u32 &j, const GPUFormat &f) {
			formats[i] = Attrib(j, i, f);
			j += u32(FormatHelper::getSizeBytes(f));
		}

		template<typename ...args>
		inline void count(const usz size, u32 &i, u32 &j, const args &...arg) {
			(registerAttrib(i, j, arg), ...);
			stride = j;
			elements = u32(size / stride);
		}

	public:

		Buffer initData{};
		List<Attrib> formats;

		GPUBuffer *buffer{};
		u32 elements, stride;

		bool isInstanced{};

		template<typename T, typename ...args>
		BufferLayout(const List<T> &b, bool isInstanced, const GPUFormat &format, const args &...arg) : 
			initData((const u8*) b.data(), (const u8*) (b.data() + b.size())), 
			formats(1 + sizeof...(arg)), isInstanced(isInstanced)
		{
			u32 i{}, j{};
			count(initData.size(), i, j, format, arg...);
		}

		template<typename T, typename ...args>
		BufferLayout(const List<T> &b, const GPUFormat &format, const args &...arg) :
			BufferLayout(b, false, format, arg...) {}

		BufferLayout(GPUBuffer *b, bool isInstanced, const List<Attrib> &formats, u32 elements, u32 stride) : 
			buffer(b), formats(formats), elements(elements), stride(stride), isInstanced(isInstanced) {  }

		BufferLayout() : formats{}, elements{}, stride{} {}

		inline const usz size() const { return usz(elements) * stride; }
	};

	class PrimitiveBuffer : public GraphicsObject {

	public:

		struct Info {

			List<BufferLayout> vertexLayout;
			BufferLayout indexLayout;
			GPUBufferUsage usage;

			Info(
				List<BufferLayout> vertexLayout, 
				BufferLayout indexLayout = {},
				GPUBufferUsage usage = GPUBufferUsage::LOCAL
			):
				vertexLayout(vertexLayout), indexLayout(indexLayout), usage(usage) { }

			Info(
				BufferLayout vertexLayout, 
				BufferLayout indexLayout = {},
				GPUBufferUsage usage = GPUBufferUsage::LOCAL
			): 
				vertexLayout({ vertexLayout }), indexLayout(indexLayout), usage(usage) {}
		};

		apimpl struct Data;

		PrimitiveBuffer(Graphics &g, const String &name, const Info &info);
		~PrimitiveBuffer();

		inline const BufferLayout &operator[](size_t i) const {
			return info.vertexLayout[i];
		}

		inline const BufferLayout &getIndexBuffer() const {
			return info.indexLayout;
		}

		inline const bool isIndexed() const {
			return info.indexLayout.formats.size();
		}

		inline const GPUBufferUsage getUsage() const {
			return info.usage;
		}

		inline const u32 indices() const {
			return info.indexLayout.elements;
		}

		inline const u32 vertices() const {
			return info.vertexLayout[0].elements;
		}

		inline const u32 elements() const {
			return isIndexed() ? indices() : vertices();
		}

		inline Data *getData() { return data; }
		inline const Info &getInfo() const { return info; }

	protected:

		apimpl void init();
		apimpl void destroy();

	private:

		Info info;
		Data *data;
	};

}