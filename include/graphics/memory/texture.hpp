#pragma once
#include "graphics/gpu_resource.hpp"
#include "graphics/enums.hpp"
#include "types/grid.hpp"

namespace ignis {
	
	class Texture : public GPUResource {

	public:

		struct Info {

			Buffer initData;

			Vec3u dimensions;
			u32 layers;

			GPUFormat format;
			GPUMemoryUsage usage;
			u8 mips;				//0 for auto, N for a specific number of mips

			TextureType textureType;

			//Empty Texture1D
			Info(
				u32 x, GPUFormat format, GPUMemoryUsage usage,
				u8 mips = 0, u32 layers = 1
			);

			//Empty Texture2D
			Info(
				Vec2u xy, GPUFormat format, GPUMemoryUsage usage,
				u8 mips = 0, u32 layers = 1
			);

			//Empty Texture3D
			Info(
				Vec3u xyz, GPUFormat format, GPUMemoryUsage usage,
				u8 mips = 0
			);

			//Filled Texture1D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const oic::Grid1D<T> &val, GPUFormat format, GPUMemoryUsage usage,
				u8 mips = 0, u32 layers = 1
			);

			//Filled Texture2D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const oic::Grid2D<T> &val, GPUFormat format, GPUMemoryUsage usage,
				u8 mips = 0, u32 layers = 1
			);

			//Filled Texture3D
			template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const oic::Grid3D<T> &val, GPUFormat format, GPUMemoryUsage usage,
				u8 mips = 0
			);

			//TODO: Cubemap
			//TODO: Store mips and layers in memory too
			//TODO: Compressed texture

			bool init(const Buffer &b);

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
		const oic::Grid1D<T> &val, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) : 
		Info(u32(val.size()), format, usage, mips, layers) {
		init(val.buffer());
	}

	template<typename T, typename>
	Texture::Info::Info(
		const oic::Grid2D<T> &val, GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) : 
		Info(
			Vec2u{ u32(val.size()[1]), u32(val.size()[0]) }, 
			format, usage, mips, layers
		) {
		init(val.buffer());
	}

	template<typename T, typename>
	Texture::Info::Info(
		const oic::Grid3D<T> &val, GPUFormat format, GPUMemoryUsage usage,
		u8 mips
	) : 
		Info(
			Vec3u{ u32(val.size()[2]), u32(val.size()[1]), u32(val.size()[0]) }, 
			format, usage, mips, 1
		) {
		init(val.buffer());
	}

}