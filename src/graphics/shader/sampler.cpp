#include "graphics/shader/sampler.hpp"

namespace ignis {

	Sampler::Info::Info(
		SamplerMin min,
		SamplerMag mag,
		SamplerMode srt,
		f32 anisotropy,
		f32 minLod,
		f32 maxLod
	) :
		Info(
			Vec4f{}, srt, srt, srt,
			min, mag, anisotropy, minLod, maxLod
		) {}

	Sampler::Info::Info(
		Vec4f borderColor,
		SamplerMin min,
		SamplerMag mag,
		f32 anisotropy,
		f32 minLod,
		f32 maxLod
	) :
		Info(
			borderColor, SamplerMode::CLAMP_BORDER,
			SamplerMode::CLAMP_BORDER, SamplerMode::CLAMP_BORDER,
			min, mag, anisotropy, minLod, maxLod
		) {}

	Sampler::Info::Info(
		SamplerMode s,
		SamplerMode r,
		SamplerMode t,
		SamplerMin min,
		SamplerMag mag,
		f32 anisotropy,
		f32 minLod,
		f32 maxLod
	) :
		Info(
			Vec4f{}, s, r, t, min, mag, anisotropy, minLod, maxLod
		) {}

	Sampler::Info::Info(
		Vec4f borderColor,
		SamplerMode s,
		SamplerMode r,
		SamplerMode t,
		SamplerMin min,
		SamplerMag mag,
		f32 anisotropy,
		f32 minLod,
		f32 maxLod
	) :
		borderColor(borderColor), s(s), r(r), t(t),
		minFilter(min), magFilter(mag), anisotropy(anisotropy),
		minLod(minLod), maxLod(maxLod) {}

	bool Sampler::isCompatible(
		const RegisterLayout &reg, const GPUSubresource &resource
	) {
		//TODO:
	}

}