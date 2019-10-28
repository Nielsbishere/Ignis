#pragma once
#include "graphics/graphics_object.hpp"
#include "pipeline_layout.hpp"

namespace ignis {

	class GPUResource;
	class GPUBuffer;

	//Describing which range of the resource has to be bound

	struct GPUSubresource {

		GPUResource *resource{};

		union {
			//TextureRange textureRange;
			//CombinedSamplerRange samplerRange;

			struct Buffer {
				usz offset{}, size {};
			} bufferRange;
		};

		GPUSubresource() : bufferRange{} {}
		GPUSubresource(GPUBuffer *resource, usz offset = 0, usz size = 0);
		//GPUResourceRange(Texture *resource): resource(resource) {}
		//GPUResourceRange(Texture *resource, Sampler *sampler)
	};

	//Which ranges of resources have to be bound

	class Descriptors : public GraphicsObject {

	public:

		using Resources = HashMap<u32, GPUSubresource>;

		struct Info {

			PipelineLayout pipelineLayout;
			Resources resources;

			bool shouldFlush{};

			Info(
				const PipelineLayout &pipelineLayout, 
				const Resources &resources
			);
		};

		apimpl struct Data;

		apimpl Descriptors(Graphics &g, const String &name, const Info &info);
		apimpl ~Descriptors();

		//Flush the updates from the CPU to the GPU
		apimpl void flush(usz offset, usz size);

		//Update the CPU-side resource; requires flush to be called afterwards
		apimpl void bindSubresource(u32 i, const GPUSubresource &range);

		//Check if descriptor slots are compatible (and exist)
		bool isResourceCompatible(u32 i, const GPUSubresource &resource) const;

		//Check if pipeline layout is compatible with a shader
		bool isShaderCompatible(const PipelineLayout &layout) const;

		Data *getData() { return data; }
		const Info &getInfo() const { return info; }

	private:

		Info info;
		Data *data{};
	};

}