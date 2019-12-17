#include "utils/hash.hpp"
#include "utils/math.hpp"
#include "utils/thread.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "graphics/surface/gl_framebuffer.hpp"
#include "graphics/gl_context.hpp"
#include "graphics/format.hpp"

namespace ignis {

	Framebuffer::Framebuffer(Graphics &g, const String &name, const Info &info): 
		Surface(g, name, info) {
		
		data = new Data();
		this->info.samples = oic::Math::min(g.getData()->maxSamples, info.samples);

		if (!info.isDynamic)
			onResize(info.size);
	}

	Framebuffer::~Framebuffer() {
		onResize(Vec2u32());
		destroy(data);
	}

	void Framebuffer::onResize(const Vec2u32 &siz) {

		Vec2u32 size = (siz.cast<Vec2f64>() * info.viewportScale).cast<Vec2u32>();

		if (info.size == size && (info.isDynamic || (!info.isDynamic && data->index)))
			return;

		info.size = size;

		if (data->index)
			glDeleteFramebuffers(1, &data->index);

		if (data->depth) {

			if (info.keepDepth)
				glDeleteTextures(1, &data->depth);
			else
				glDeleteRenderbuffers(1, &data->depth);
		}

		for (GLuint &renderTexture : data->renderTextures)
			if (renderTexture)
				glDeleteTextures(1, &renderTexture);

		if (!size.x) {
			data->index = data->depth = 0;
			return;
		}

		glCreateFramebuffers(1, &data->index);
		GLuint fb = data->index;

		glObjectLabel(
			GL_FRAMEBUFFER, fb, GLsizei(getName().size()), getName().c_str()
		);

		bool fixedSampleLocations = info.depthFormat != DepthFormat::NONE && !info.keepDepth;

		if (info.depthFormat != DepthFormat::NONE) {

			//TODO: Stencil buffers

			if (info.keepDepth) {

				glCreateTextures(
					glxTextureType(getTextureType()), 1, &data->depth
				);

				GLuint handle = data->depth;

				String hashed = NAME(getName() + " depth texture");
				glObjectLabel(
					GL_TEXTURE, handle, GLsizei(hashed.size()), hashed.c_str()
				);

				glTextureStorage2DMultisample(
					handle, GLsizei(info.samples),
					glxDepthFormat(info.depthFormat), size.x, size.y, GL_FALSE
				);

				glNamedFramebufferTexture(fb, GL_DEPTH_ATTACHMENT, handle, 0);

			} else {

				//TODO: Fix render buffers!

				glCreateRenderbuffers(1, &data->depth);
				GLuint handle = data->depth;

				String hashed = NAME(getName() + " depth buffer");
				glObjectLabel(
					GL_RENDERBUFFER, handle, GLsizei(hashed.size()), hashed.c_str()
				);

				glNamedRenderbufferStorageMultisample(
					handle, GLsizei(info.samples),
					glxDepthFormat(info.depthFormat), size.x, size.y
				);

				glNamedFramebufferRenderbuffer(
					fb,
					FormatHelper::hasStencil(info.depthFormat) ? 
					GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, 
					GL_RENDERBUFFER, handle
				);
			}
		}

		data->renderTextures = List<GLuint>(info.colorFormats.size());
		List<GLenum> drawBuffers(info.colorFormats.size());

		glCreateTextures(
			glxTextureType(getTextureType()), 
			GLsizei(info.colorFormats.size()), data->renderTextures.data()
		);

		for (usz i = 0, j = info.colorFormats.size(); i < j; ++i) {

			GLuint tex = data->renderTextures[i];

			glTextureStorage2DMultisample(
				tex, GLsizei(info.samples),
				glxColorFormat(info.colorFormats[i]), size.x, size.y, fixedSampleLocations
			);

			String hashed = NAME(getName() + " buffer " + std::to_string(i));
			glObjectLabel(GL_TEXTURE, tex, GLsizei(hashed.size()), hashed.c_str());

			glNamedFramebufferTexture(fb, drawBuffers[i] = GL_COLOR_ATTACHMENT0 + GLenum(i), tex, 0);
		}

		glNamedFramebufferDrawBuffers(fb, GLsizei(drawBuffers.size()), drawBuffers.data());
		GLenum status = glCheckNamedFramebufferStatus(fb, GL_FRAMEBUFFER);

		if (status != GL_FRAMEBUFFER_COMPLETE)
			oic::System::log()->fatal("Couldn't create framebuffer");

	}

	void Framebuffer::begin() {
		glxBeginRenderPass(
			getGraphics().getData()->getContext(), data->index
		);
	}

	void Framebuffer::end() {}

}