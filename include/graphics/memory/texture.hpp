#pragma once
#include "graphics/gpu_resource.hpp"
#include "graphics/enums.hpp"
#include "types/grid.hpp"

namespace ignis {
	
	class Texture : public GPUResource {

	public:

		struct Info {

			List<Buffer> initData;		//[mip] Where each buffer.size = linearSize*layers

			Vec3u dimensions;
			u32 layers;

			GPUFormat format;
			GPUMemoryUsage usage;
			u8 mips;

			TextureType textureType;

			//Empty Texture1D
			Info(
				u32 x, GPUFormat format, GPUMemoryUsage usage,
				u8 mips, u32 layers = 1
			);

			//Empty Texture2D
			Info(
				Vec2u xy, GPUFormat format, GPUMemoryUsage usage,
				u8 mips, u32 layers = 1
			);

			//Empty Texture3D
			Info(
				Vec3u xyz, GPUFormat format,
				GPUMemoryUsage usage, u8 mips
			);

			//Filled Texture1D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const List<oic::Grid1D<T>> &val, GPUFormat format,
				GPUMemoryUsage usage, u8 mips, u32 layers = 1
			);

			//Filled Texture2D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const List<oic::Grid2D<T>> &val, GPUFormat format,
				GPUMemoryUsage usage, u8 mips, u32 layers = 1
			);

			//Filled Texture3D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const List<oic::Grid3D<T>> &val, GPUFormat format, 
				GPUMemoryUsage usage, u8 mips
			);

			//TODO: Cubemap
			//TODO: Compressed texture

			//List<Buffer> with size mips with each buffer of size layers * multiplied(resolution) * stride
			bool init(const List<Buffer> &b);

		private:

			Info(
				TextureType textureType,
				Vec3u xyz, GPUFormat format, GPUMemoryUsage usage,
				u8 mipCount, u32 layers
			);
		};

		apimpl struct Data;

		apimpl Texture(Graphics &g, const String &name, const Info &info);
		apimpl ~Texture();

		bool isCompatible(
			const RegisterLayout &reg, const GPUSubresource &resource
		) const final override;

		bool validSubresource(
			const GPUSubresource &resource, bool isSampler = false
		) const;

		Data *getData() { return data; }
		const Info &getInfo() const { return info; }

	private:

		Info info;
		Data *data;
	};

	template<typename T, typename>
	Texture::Info::Info(
		const List<oic::Grid1D<T>> &val, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) : 
		Info(u32(val[0].size()), format, usage, mips, layers) {

		if (dimensions[0] % layers != 0)
			oic::System::log()->fatal("Width of Grid1D should encompass layer count");

		dimensions[0] /= layers;

		List<Buffer> buffers(val.size());

		for (usz i = 0, j = val.size(); i < j; ++i)
			buffers[i] = val[i].buffer();

		init(buffers);
	}

	template<typename T, typename>
	Texture::Info::Info(
		const List<oic::Grid2D<T>> &val, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) : 
		Info(
			Vec2u{ u32(val[0].size()[1]), u32(val[0].size()[0]) }, 
			format, usage, mips, layers
		) {

		if (dimensions[1] % layers != 0)
			oic::System::log()->fatal("Height of Grid2D should encompass layer count");

		dimensions[1] /= layers;

		List<Buffer> buffers(val.size());

		for (usz i = 0, j = val.size(); i < j; ++i)
			buffers[i] = val[i].buffer();

		init(buffers);
	}

	template<typename T, typename>
	Texture::Info::Info(
		const List<oic::Grid3D<T>> &val, GPUFormat format, GPUMemoryUsage usage,
		u8 mips
	) : 
		Info(
			Vec3u{ u32(val[0].size()[2]), u32(val[0].size()[1]), u32(val[0].size()[0]) }, 
			format, usage, mips, 1
		) {

		List<Buffer> buffers(val.size());

		for (usz i = 0, j = val.size(); i < j; ++i)
			buffers[i] = val[i].buffer();

		init(buffers);
	}

}