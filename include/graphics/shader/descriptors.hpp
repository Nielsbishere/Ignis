#pragma once
#include "graphics/graphics.hpp"
#include "pipeline_layout.hpp"
#include "types/vec.hpp"

namespace ignis {

	class GPUResource;
	class GPUBuffer;
	class Sampler;
	class Texture;
	class TextureObject;

	//Describing which range of the resource has to be bound

	struct GPUSubresource {

		GPUResource *resource{};

		struct TextureRange {

			u32 minLevel{}, minLayer{};
			u32 levelCount{}, layerCount{};
			TextureType subType{};

			TextureRange() {}
			TextureRange(
				u32 minLevel, u32 minLayer,
				u32 levelCount, u32 layerCount,
				TextureType subType
			) :
				minLevel(minLevel), minLayer(minLayer), 
				levelCount(levelCount), layerCount(layerCount), subType(subType) {}

			inline bool operator==(const TextureRange &other) const {
				return
					*(const u64*)&minLevel == *(const u64*)&other.minLevel && 
					*(const u64*)&levelCount == *(const u64*)&other.levelCount &&
					subType == other.subType;
			}
		};

		struct SamplerData : public TextureRange {

			TextureObject *texture{};

			SamplerData() {}
			SamplerData(
				TextureObject *texture,
				u32 minLevel, u32 minLayer,
				u32 levelCount, u32 layerCount,
				TextureType subType
			) :
				TextureRange(minLevel, minLayer, levelCount, layerCount, subType),
				texture(texture) {}
		};

		struct BufferRange {

			usz offset{}, size{};

			BufferRange() {}
			BufferRange(usz offset, usz size): offset(offset), size(size) {}
		};

		union {
			TextureRange textureRange;
			SamplerData samplerData;
			BufferRange bufferRange;
		};

		GPUSubresource(): samplerData{} {}
		GPUSubresource(GPUBuffer *resource, usz offset = 0, usz size = 0);

		GPUSubresource(
			Sampler *sampler, TextureObject *texture,
			TextureType subType,
			u32 levelCount = 0, u32 layerCount = 0,
			u32 minLevel = 0, u32 minLayer = 0
		);

		GPUSubresource(Sampler *resource);

		GPUSubresource(
			TextureObject *resource, TextureType subType,
			u32 levelCount = 0, u32 layerCount = 0,
			u32 minLevel = 0, u32 minLayer = 0
		);
	};

	//Which ranges of resources have to be bound

	class Descriptors : public GPUObject {

	public:

		using Subresources = HashMap<u32, GPUSubresource>;

		struct Info {

			const PipelineLayout *pipelineLayout;
			Subresources resources, flushedResources;

			u16 descriptorSetIndex{};
			bool shouldFlush{};

			Info(
				const PipelineLayout *pipelineLayout,
				u16 descriptorIndex,
				const Subresources &resources
			);
		};

		apimpl struct Data;

		apimpl Descriptors(Graphics &g, const String &name, const Info &info);

		//Flush the updates from the CPU to the GPU
		apimpl void flush(const List<Vec2u32> &offsetsAndSizes);

		//Update the CPU-side resource; requires flush to be called afterwards
		apimpl void updateDescriptor(u32 i, const GPUSubresource &range);

		//Check if descriptor slots are compatible (and exist)
		bool isResourceCompatible(u32 i, const GPUSubresource &resource) const;

		Data *getData() { return data; }
		const Info &getInfo() const { return info; }

	private:

		apimpl ~Descriptors();

		Info info;
		Data *data{};
	};

	using DescriptorsRef = GraphicsObjectRef<Descriptors>;
}