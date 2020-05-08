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

		protected:

			Info(
				TextureType textureType,
				const Vec3u16 &xyz, GPUFormat format, GPUMemoryUsage usage,
				u8 mipCount, u16 layers, u8 samples, bool useFixedSampleLocations
			);

			Info(
				TextureType textureType, GPUFormat format, GPUMemoryUsage usage,
				u8 mipCount, u16 layers, u8 samples, bool useFixedSampleLocations
			);
		};

		~TextureObject() {}

		virtual TextureObjectType getTextureObjectType() const = 0;

		//If a register (and subresource) are compatible
		bool isCompatible(
			const RegisterLayout &reg, const GPUSubresource &resource
		) const override;

		//If a subresource is compatible with this texture
		bool validSubresource(
			const GPUSubresource &resource, bool isSampler = false
		) const;

		//If a subtype for a texture view can be used for this texture type
		bool isValidSubType(
			const TextureType type
		) const;

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