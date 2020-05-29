#pragma once
#include "graphics/memory/texture_object.hpp"
#include "graphics/command/command_list.hpp"
#include "types/grid.hpp"

namespace ignis {

	template<typename T>
	using DataTexture1D = List<oic::Grid1D<T>>;		//If layered: width / layers = imageWidth

	template<typename T>
	using DataTexture2D = List<oic::Grid2D<T>>;		//If layered: height / layers = imageHeight

	template<typename T>
	using DataTexture3D = List<oic::Grid3D<T>>;

	class Texture : public TextureObject {

		friend class CommandList;

	public:

		struct Info : public TextureObject::Info {

			//Each mip is stored as an index into the list
			//Each layer is stored in the grid
			//[mip] Where each buffer.size = linearSize*layers
			List<Buffer> initData;

			//

			using TextureObject::Info::Info;

			//Filled Texture1D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const DataTexture1D<T> &val, GPUFormat format,
				GPUMemoryUsage usage, u16 layers
			);

			//Filled Texture2D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const DataTexture2D<T> &val, GPUFormat format,
				GPUMemoryUsage usage, u16 layers
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
				GPUMemoryUsage usage, bool isCubeArray, u16 layers /* Should be a multiple of 6 (or 6 if !isCubeArray) */
			);

			//TODO: Compressed texture

			//List<Buffer> with a buffer per mip with each buffer of size layers * multiplied(resolution) * stride
			bool init(const List<Buffer> &b);
		};

		apimpl Texture(Graphics &g, const String &name, const Info &info);

		inline const Info &getInfo() const { return info; }
		inline u8 *getTextureData(usz mip = 0) { return info.initData[mip].data(); }

		template<typename T>
		inline oic::Grid1D<T> getTextureData1D(usz mip = 0);

		template<typename T>
		inline oic::Grid2D<T> getTextureData2D(usz mip = 0);

		template<typename T>
		inline oic::Grid3D<T> getTextureData3D(usz mip = 0);

		inline TextureObjectType getTextureObjectType() const final override {
			return TextureObjectType::TEXTURE;
		}

	private:

		apimpl void flush(CommandList::Data *commandList, UploadBuffer *uploadBuffer, u8 mip, u8 mipCount);

		apimpl ~Texture();

		Info info;
	};

	template<typename T, typename>
	Texture::Info::Info(
		const DataTexture1D<T> &val, GPUFormat format,
		GPUMemoryUsage usage, u16 layers
	) : 
		Info(u16(val[0].size() / layers), format, usage, u8(val.size()), layers)
	{

		if (val[0].size() % layers != 0)
			oic::System::log()->fatal("Width of Grid1D should encompass layer count");

		if(val[0].size() / layers > u16_MAX)
			oic::System::log()->fatal("Width of texture has to be less than 65536");

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
		GPUMemoryUsage usage, u16 layers
	) : 
		Info(
			Vec2u16(u16(val[0].size()[1]), u16(val[0].size()[0] / layers)), 
			format, usage, u8(val.size()), layers
		)
	{

		if (val[0].size()[0] % layers != 0)
			oic::System::log()->fatal("Height of Grid2D should encompass layer count");

		if(val[0].size()[0] / layers > u16_MAX || val[0].size()[1] > u16_MAX)
			oic::System::log()->fatal("Dimensions of texture have to be less than 65536");

		if(val.size() > u8_MAX)
			oic::System::log()->fatal("Texture has too many mips");

		List<Buffer> buffers(val.size());

		for (usz i = 0, j = val.size(); i < j; ++i)
			buffers[i] = val[i].buffer();

		init(buffers);
	}

	template<typename T, typename>
	Texture::Info::Info(
		const DataTexture3D<T> &val,
		GPUFormat format, GPUMemoryUsage usage
	) : 
		Info(
			Vec3u16{ u16(val[0].size()[2]), u16(val[0].size()[1]), u16(val[0].size()[0]) }, 
			format, usage, u8(val.size()), 1
		)
	{

		if((val[0].size() > u16_MAX).any())
			oic::System::log()->fatal("Dimensions of texture have to be less than 65536");

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
		GPUMemoryUsage usage, bool isCubeArray, u16 layers
	):
		Info(
			isCubeArray ? TextureType::TEXTURE_CUBE_ARRAY : TextureType::TEXTURE_CUBE, 
			Vec3u16{ u16(val[0].size()[1]), u16(val[0].size()[0] / layers), 1 }, 
			format, usage, u8(val.size()), layers
		)
	{

		if (val[0].size()[0] % layers != 0)
			oic::System::log()->fatal("Height of Grid2D should encompass layer count");

		if(val[0].size()[0] / layers > u16_MAX || val[0].size()[1] > u16_MAX)
			oic::System::log()->fatal("Dimensions of texture have to be less than 65536");

		if ((!isCubeArray && layers != 6) || (isCubeArray && layers % 6 != 0))
			oic::System::log()->fatal("Cubemaps require 6 layers per cubemap. Only cube arrays support multiple cubes");

		if(val.size() > u8_MAX)
			oic::System::log()->fatal("Texture has too many mips");
	}

	template<typename T>
	inline oic::Grid1D<T> Texture::getTextureData1D(usz mip) {

		if(!u8(info.usage & GPUMemoryUsage::CPU_WRITE))
			oic::System::log()->fatal("Can't get texture data of a non CPU writable texture");

		if (info.textureType != TextureType::TEXTURE_1D)
			oic::System::log()->fatal("Can't get 1D texture data of a non 1D texture");

		return oic::Grid1D<T>(info.initData[mip].data(), info.initData[mip].size());
	}

	template<typename T>
	inline oic::Grid2D<T> Texture::getTextureData2D(usz mip) {

		if(!u8(info.usage & GPUMemoryUsage::CPU_WRITE))
			oic::System::log()->fatal("Can't get texture data of a non CPU writable texture");

		if (
			info.textureType != TextureType::TEXTURE_2D && 
			info.textureType != TextureType::TEXTURE_1D_ARRAY
		)
			oic::System::log()->fatal("Can't get 2D texture data of a non 2D texture");

		return oic::Grid2D<T>(info.initData[mip].data(), info.initData[mip].size(), info.mipSizes[mip].x);
	}

	template<typename T>
	inline oic::Grid3D<T> Texture::getTextureData3D(usz mip) {

		if(!u8(info.usage & GPUMemoryUsage::CPU_WRITE))
			oic::System::log()->fatal("Can't get texture data of a non CPU writable texture");

		if (
			info.textureType == TextureType::TEXTURE_1D ||
			info.textureType == TextureType::TEXTURE_2D || 
			info.textureType == TextureType::TEXTURE_1D_ARRAY
		)
			oic::System::log()->fatal("Can't get 3D texture data of a non 3D texture");

		return oic::Grid3D<T>(
			info.initData[mip].data(), info.initData[mip].size(), info.mipSizes[mip].xy().cast<Vec2usz>()
		);
	}

}