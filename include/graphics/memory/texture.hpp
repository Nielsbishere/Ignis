#pragma once
#include "graphics/gpu_resource.hpp"
#include "graphics/enums.hpp"
#include "types/grid.hpp"

namespace ignis {
	
	class Texture : public GPUResource {

	public:

		struct Info {

			//Each mip is stored as an index into the list
			//Each layer is stored in the grid

			template<typename T>
			using DataTexture1D = List<oic::Grid1D<T>>;		//If layered: width / layers = imageWidth

			template<typename T>
			using DataTexture2D = List<oic::Grid2D<T>>;		//If layered: height / layers = imageHeight

			template<typename T>
			using DataTexture3D = List<oic::Grid3D<T>>;

			//Data

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
				u8 mips, u32 layers
			);

			//Empty Texture2D
			Info(
				Vec2u xy, GPUFormat format, GPUMemoryUsage usage,
				u8 mips, u32 layers
			);

			//Empty Texture3D
			Info(
				Vec3u xyz, GPUFormat format,
				GPUMemoryUsage usage, u8 mips
			);

			//Filled Texture1D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const DataTexture1D<T> &val, GPUFormat format,
				GPUMemoryUsage usage, u32 layers
			);

			//Filled Texture2D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const DataTexture2D<T> &val, GPUFormat format,
				GPUMemoryUsage usage, u32 layers
			);

			//Filled Texture3D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const DataTexture3D<T> &val, GPUFormat format, 
				GPUMemoryUsage usage
			);

			//Cubemap
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const DataTexture2D<T> &val, GPUFormat format,
				GPUMemoryUsage usage, bool isCubeArray, u32 layers /* Should be a multiple of 6 (or 6 if !isCubeArray) */
			);

			//TODO: Compressed texture

			//List<Buffer> with a buffer per mip with each buffer of size layers * multiplied(resolution) * stride
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

		//If a register (and subresource) are compatible
		bool isCompatible(
			const RegisterLayout &reg, const GPUSubresource &resource
		) const final override;

		//If a subresource is compatible with this texture
		bool validSubresource(
			const GPUSubresource &resource, bool isSampler = false
		) const;

		//If a subtype for a texture view can be used for this texture type
		bool isValidSubType(
			const TextureType type
		) const;

		Data *getData() { return data; }
		const Info &getInfo() const { return info; }

	private:

		Info info;
		Data *data;
	};

	template<typename T, typename>
	Texture::Info::Info(
		const DataTexture1D<T> &val, GPUFormat format, GPUMemoryUsage usage, u32 layers
	) : 
		Info(u32(val[0].size()), format, usage, u8(val.size()), layers) {

		if (dimensions[0] % layers != 0)
			oic::System::log()->fatal("Width of Grid1D should encompass layer count");

		if(val.size() > u8_MAX)
			oic::System::log()->fatal("Texture has too many mips");

		dimensions[0] /= layers;

		List<Buffer> buffers(val.size());

		for (usz i = 0, j = val.size(); i < j; ++i)
			buffers[i] = val[i].buffer();

		init(buffers);
	}

	template<typename T, typename>
	Texture::Info::Info(
		const DataTexture2D<T> &val, GPUFormat format, GPUMemoryUsage usage, u32 layers
	) : 
		Info(
			Vec2u{ u32(val[0].size()[1]), u32(val[0].size()[0]) }, 
			format, usage, u8(val.size()), layers
		) {

		if (dimensions[1] % layers != 0)
			oic::System::log()->fatal("Height of Grid2D should encompass layer count");

		if(val.size() > u8_MAX)
			oic::System::log()->fatal("Texture has too many mips");

		dimensions[1] /= layers;

		List<Buffer> buffers(val.size());

		for (usz i = 0, j = val.size(); i < j; ++i)
			buffers[i] = val[i].buffer();

		init(buffers);
	}

	template<typename T, typename>
	Texture::Info::Info(
		const DataTexture3D<T> &val, GPUFormat format, GPUMemoryUsage usage
	) : 
		Info(
			Vec3u{ u32(val[0].size()[2]), u32(val[0].size()[1]), u32(val[0].size()[0]) }, 
			format, usage, u8(val.size()), 1
		) {

		if(val.size() > u8_MAX)
			oic::System::log()->fatal("Texture has too many mips");

		List<Buffer> buffers(val.size());

		for (usz i = 0, j = val.size(); i < j; ++i)
			buffers[i] = val[i].buffer();

		init(buffers);
	}

	template<typename T, typename>
	Texture::Info::Info(
		const DataTexture2D<T> &val, GPUFormat format,
		GPUMemoryUsage usage, bool isCubeArray, u32 layers
	):
		Info(
			isCubeArray ? TextureType::TEXTURE_CUBE_ARRAY : TextureType::TEXTURE_CUBE, 
			Vec3u{ u32(val[0].size()[1]), u32(val[0].size()[0]) }, 
			format, usage, u8(val.size()), layers
		) {

		if (dimensions[1] % layers != 0)
			oic::System::log()->fatal("Height of Grid2D should encompass layer count");

		if ((!isCubeArray && layers != 6) || (isCubeArray && layers % 6 != 0))
			oic::System::log()->fatal("Cubemaps require 6 layers per cubemap. Only cube arrays support multiple cubes");

		if(val.size() > u8_MAX)
			oic::System::log()->fatal("Texture has too many mips");
	}

}