#pragma once
#include "graphics/graphics_object.hpp"
#include "pipeline_layout.hpp"

namespace ignis {

	class GPUResource;

	class Descriptors : public GraphicsObject {

	public:

		struct Info {

			PipelineLayout pipelineLayout;
			HashMap<u32, GPUResource*> resources;

			bool shouldFlush{};

			Info(const PipelineLayout &pipelineLayout, const HashMap<u32, GPUResource*> &resources);
		};

		apimpl struct Data;

		apimpl Descriptors(Graphics &g, const String &name, const Info &info);
		apimpl ~Descriptors();

		//Flush the updates from the CPU to the GPU
		apimpl void flush(usz offset, usz size);

		//Update the CPU-side resource; requires flush to be called afterwards
		apimpl void setResource(u32 i, GPUResource *resource);

		//Check if descriptor slots are compatible (and exist)
		bool isResourceCompatible(u32 i, GPUResource *resource) const;

		//Check if pipeline layout is compatible with a shader
		bool isShaderCompatible(const PipelineLayout &layout) const;

		Data *getData() { return data; }
		const Info &getInfo() const { return info; }

	private:

		Info info;
		Data *data{};
	};

}