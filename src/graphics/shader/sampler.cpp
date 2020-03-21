#include "graphics/shader/sampler.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/memory/texture.hpp"

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
			Vec4f32(), srt, srt, srt,
			min, mag, anisotropy, minLod, maxLod
		) {}

	Sampler::Info::Info(
		const Vec4f32 &borderColor,
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
			Vec4f32(), s, r, t, min, mag, anisotropy, minLod, maxLod
		) {}

	Sampler::Info::Info(
		const Vec4f32 &borderColor,
		SamplerMode s,
		SamplerMode r,
		SamplerMode t,
		SamplerMin min,
		SamplerMag mag,
		f32 anisotropy,
		f32 minLod,
		f32 maxLod
	) :
		borderColor(borderColor), anisotropy(anisotropy),
		minLod(minLod), maxLod(maxLod),
		minFilter(min), magFilter(mag), s(s), r(r), t(t) {}

	bool Sampler::isCompatible(
		const RegisterLayout &rl, const GPUSubresource &sub
	) const {

		TextureObject *tex = sub.samplerData.texture;

		if (rl.type == ResourceType::SAMPLER)
			return !tex;

		else if(rl.type != ResourceType::COMBINED_SAMPLER)
			return false;

		auto samplerTexture = TextureType(rl.samplerType & SamplerType::PROPERTY_AS_TEXTURE);

		return 
			tex && 
			tex->getInfo().textureType == samplerTexture &&
			tex->validSubresource(sub, true);
	}

}