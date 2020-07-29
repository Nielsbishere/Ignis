#pragma once
#include "graphics/gpu_resource.hpp"
#include "graphics/enums.hpp"
#include "types/vec.hpp"

namespace ignis {
	
	enum class TextureObjectType : u8 {
		TEXTURE,
		RENDER,
		DEPTH
	};

	//If specified on a Texture2DArray, the z of start is a layer
	struct TextureRange { Vec3u16 start, size; u8 mip; };

	//An object that stores array texel data
	class TextureObject : public GPUObject, public GPUResource {

	public:

		struct Info {

			friend class Framebuffer;

			Vec3u16 dimensions;

			GPUFormat format;
			GPUMemoryUsage usage;
			u8 mips;

			u16 layers;
			TextureType textureType;
			u8 samples;

			bool useFixedSampleLocations;

			List<Vec3u16> mipSizes;

			Info(
				u16 res, GPUFormat format, GPUMemoryUsage usage,
				u8 mips, u16 layers
			);

			Info(
				const Vec2u16 &res, GPUFormat format, GPUMemoryUsage usage,
				u8 mips, u16 layers
			);

			Info(
				const Vec2u16 &res, GPUFormat format, GPUMemoryUsage usage,
				u8 mips, u16 layers, u8 samples, bool useFixedSampleLocations
			);

			Info(
				const Vec3u16 &res, GPUFormat format,
				GPUMemoryUsage usage, u8 mips
			);

			Info(
				TextureType textureType,
				const Vec3u16 &xyz, GPUFormat format, GPUMemoryUsage usage,
				u8 mipCount, u16 layers, u8 samples, bool useFixedSampleLocations
			);

		protected:

			Info(
				TextureType textureType, GPUFormat format, GPUMemoryUsage usage,
				u8 mipCount, u16 layers, u8 samples, bool useFixedSampleLocations
			);
		};

		~TextureObject() {}

		virtual TextureObjectType getTextureObjectType() const = 0;

		bool isCompatible(const RegisterLayout &reg, const GPUSubresource &resource) const override;

		bool isValidSubresource(const GPUSubresource &resource, bool isSampler = false) const;
		bool isValidSubType(const TextureType type) const;
		bool isValidRange(const TextureRange &range) const;

		//Gets the size in bytes of this texture objects on the GPU
		//"isStencil" represents if the stencil is being queried, this is only valid for depth textures
		usz size(u8 mip = 0, u16 layer = 0, u16 depth = 0, bool isStencil = false) const;

		//Gets dimensions of this texture (with the layer in the correct slot)
		Vec3u16 getDimensions(u8 mip) const;

		//Get the index in the dimension array that MIGHT contain the layer
		//if HasFlags(info.textureType, TextureType::PROPERTY_IS_ARRAY) is false, it doesn't
		//	and might contain the z of y instead
		usz getDimensionLayerId() const;

		inline const Info &getInfo() const { return info; }

		plimpl struct Data;
		inline Data *getData() const { return data; }

	protected:

		TextureObject(Graphics &g, const String &name, const Info &info, const GPUObjectType objectType): 
			GPUObject(g, name, objectType), info(info) {}

		Info info;
		Data *data;
	};

}