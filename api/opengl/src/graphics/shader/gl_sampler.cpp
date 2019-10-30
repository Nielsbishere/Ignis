#include "graphics/shader/gl_sampler.hpp"
#include "utils/math.hpp"

namespace ignis {

	Sampler::Sampler(Graphics &g, const String &name, const Info &inf):
		GPUResource(g, name), info(inf) {

		info.anisotropy = oic::Math::min(
			g.getData()->maxAnistropy, inf.anisotropy
		);

		data = new Data();

		glCreateSamplers(1, &data->handle);
		GLuint h = data->handle;

		glSamplerParameteri(h, GL_TEXTURE_WRAP_R,		glxSamplerMode(inf.r));
		glSamplerParameteri(h, GL_TEXTURE_WRAP_S,		glxSamplerMode(inf.s));
		glSamplerParameteri(h, GL_TEXTURE_WRAP_T,		glxSamplerMode(inf.t));
		glSamplerParameteri(h, GL_TEXTURE_MAG_FILTER,	glxSamplerMag(inf.magFilter));
		glSamplerParameteri(h, GL_TEXTURE_MIN_FILTER,	glxSamplerMin(inf.minFilter));

		glSamplerParameterf(h, GL_TEXTURE_MAX_ANISOTROPY,	info.anisotropy);
		glSamplerParameterf(h, GL_TEXTURE_MIN_LOD,			inf.minLod);
		glSamplerParameterf(h, GL_TEXTURE_MAX_LOD,			inf.maxLod);

		glSamplerParameterfv(h, GL_TEXTURE_BORDER_COLOR,	inf.borderColor.data());
	}

	Sampler::~Sampler() {
		glDeleteSamplers(1, &data->handle);
		delete data;
	}

}