#include "graphics/shader/gl_sampler.hpp"
#include "utils/math.hpp"

namespace ignis {

	Sampler::Sampler(Graphics &g, const String &name, const Info &inf):
		GPUObject(g, name, GPUObjectType::SAMPLER), info(inf) {

		info.anisotropy = oic::Math::min(
			g.getData()->maxAnistropy, inf.anisotropy
		);

		data = new Data();

		glCreateSamplers(1, &data->handle);
		GLuint h = data->handle;

		glObjectLabel(GL_SAMPLER, h, GLsizei(getName().size()), getName().c_str());

		if(inf.r != SamplerMode::REPEAT)
			glSamplerParameteri(h, GL_TEXTURE_WRAP_R, glxSamplerMode(inf.r));

		if(inf.s != SamplerMode::REPEAT)
			glSamplerParameteri(h, GL_TEXTURE_WRAP_S, glxSamplerMode(inf.s));

		if(inf.t != SamplerMode::REPEAT)
			glSamplerParameteri(h, GL_TEXTURE_WRAP_T, glxSamplerMode(inf.t));

		if(inf.magFilter != SamplerMag::LINEAR)
			glSamplerParameteri(h, GL_TEXTURE_MAG_FILTER, glxSamplerMag(inf.magFilter));

		if(inf.minFilter != SamplerMin::NEAREST_MIPS_LINEAR)
			glSamplerParameteri(h, GL_TEXTURE_MIN_FILTER, glxSamplerMin(inf.minFilter));

		if(inf.anisotropy > 1)
			glSamplerParameterf(h, GL_TEXTURE_MAX_ANISOTROPY, info.anisotropy);

		if(inf.minLod != -f32_MAX && inf.minLod != -1000)
			glSamplerParameterf(h, GL_TEXTURE_MIN_LOD, inf.minLod);

		if(inf.maxLod != f32_MAX && inf.maxLod != 1000)
			glSamplerParameterf(h, GL_TEXTURE_MAX_LOD, inf.maxLod);
	}

	Sampler::~Sampler() {
		glDeleteSamplers(1, &data->handle);
		destroy(data);
	}

}