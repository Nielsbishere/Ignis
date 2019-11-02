#pragma once
#include "graphics/gpu_resource.hpp"
#include "graphics/enums.hpp"

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
				u8 mips = 0, u32 layers = 1
			);

			//Filled Texture1D
			template<typename T, u32 W, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const T (&val)[W], GPUFormat format, GPUMemoryUsage usage,
				u8 mips = 0, u32 layers = 1
			);

			//Filled Texture2D
			template<typename T, u32 W, u32 H, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const T (&val)[W][H], GPUFormat format, GPUMemoryUsage usage,
				u8 mips = 0, u32 layers = 1
			);

			//Filled Texture3D
			template<typename T, u32 W, u32 H, u32 L, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
			Info(
				const T (&val)[W][H][L], GPUFormat format, GPUMemoryUsage usage,
				u8 mips = 0, u32 layers = 1
			);

			//TODO: Cubemap
			//TODO: Grid3D/Grid2D/Grid1D allocation (dynamic)
			//TODO: Store mips and layers in memory too
			//TODO: Compressed texture

			bool init(const Buffer &b);
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

	template<typename T, u32 W, typename>
	Texture::Info::Info(
		const T (&val)[W], GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) : 
		Info(W, format, usage, mips, layers) {
		init(Buffer((u8*)val, ((u8*)val) + sizeof(val)));
	}

	template<typename T, u32 W, u32 H, typename>
	Texture::Info::Info(
		const T (&val)[W][H], GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) : 
		Info(Vec2u{ W, H }, format, usage, mips, layers) {
		init(Buffer((u8*)val, ((u8*)val) + sizeof(val)));
	}

	template<typename T, u32 W, u32 H, u32 L, typename>
	Texture::Info::Info(
		const T (&val)[W][H][L], GPUFormat format, GPUMemoryUsage usage,
		u8 mips, u32 layers
	) : 
		Info(Vec3u{ W, H, L }, format, usage, mips, layers) {
		init(Buffer((u8*)val, ((u8*)val) + sizeof(val)));
	}

}