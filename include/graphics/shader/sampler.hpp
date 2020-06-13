#pragma once
#include "graphics/gpu_resource.hpp"
#include "graphics/enums.hpp"
#include "types/vec.hpp"

namespace ignis {

	class Sampler : public GPUObject, public GPUResource {

	public:

		struct Info {

			f32 anisotropy, minLod, maxLod;

			SamplerMin minFilter;
			SamplerMag magFilter;

			SamplerMode s, r, t;

			Info(
				SamplerMin min = SamplerMin::LINEAR_MIPS,
				SamplerMag mag = SamplerMag::LINEAR,
				SamplerMode srtFilterMode = SamplerMode::REPEAT,
				f32 anisotropy = 8,
				f32 minLod = -f32_MAX,
				f32 maxLod = f32_MAX
			);

			Info(
				SamplerMode sFilter,
				SamplerMode rFilter,
				SamplerMode tFilter,
				SamplerMin min = SamplerMin::LINEAR_MIPS,
				SamplerMag mag = SamplerMag::LINEAR,
				f32 anisotropy = 8,
				f32 minLod = -f32_MAX,
				f32 maxLod = f32_MAX
			);
		};

		apimpl struct Data;

		apimpl Sampler(Graphics &g, const String &name, const Info &info);

		bool isCompatible(
			const RegisterLayout &reg, const GPUSubresource &resource
		) const final override;

		Data *getData() { return data; }
		const Info &getInfo() const { return info; }

	private:

		apimpl ~Sampler();

		Info info;
		Data *data;
	};

	using SamplerRef = GraphicsObjectRef<Sampler>;
}