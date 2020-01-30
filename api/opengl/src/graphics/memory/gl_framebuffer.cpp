#include "utils/hash.hpp"
#include "utils/math.hpp"
#include "utils/thread.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "graphics/memory/gl_framebuffer.hpp"
#include "graphics/memory/render_texture.hpp"
#include "graphics/memory/depth_texture.hpp"
#include "graphics/memory/gl_texture_object.hpp"
#include "graphics/gl_context.hpp"
#include "graphics/format.hpp"

namespace ignis {

	Framebuffer::Framebuffer(Graphics &g, const String &name, const Info &inf): 
		GraphicsObject(g, name), info(inf) {
		
		data = new Data();
		info.samples = oic::Math::min(g.getData()->maxSamples, info.samples);

		if (info.depthFormat != DepthFormat::NONE)
			depth = new DepthTexture(
				g, name + NAME(" depth buffer"), 
				DepthTexture::Info(
					info.depthFormat, info.keepDepth, GPUMemoryUsage::LOCAL, 1, 1, info.samples, false
				)
			);

		usz i{};
		targets.resize(info.colorFormats.size());

		for (auto col : info.colorFormats)
			if (col == GPUFormat::NONE)
				oic::System::log()->fatal("GPUFormat can't be NONE for a framebuffer");
			else {
				targets[i] = new RenderTexture(
					g, NAME(name + " target " + oic::Log::num(i)),
					TextureObject::Info(
						info.samples > 1 ? TextureType::TEXTURE_MS : TextureType::TEXTURE_2D,
						col, GPUMemoryUsage::LOCAL,
						1, 1,
						info.samples, info.depthFormat != DepthFormat::NONE && !info.keepDepth
					)
				);
				++i;
			}

		if (!info.isDynamic)
			onResize(info.size.cast<Vec2u32>());
	}

	Framebuffer::~Framebuffer() {

		onResize(Vec2u32());

		depth->loseRef();

		for (auto *target : targets)
			target->loseRef();

		destroy(data);
	}

	void Framebuffer::onResize(const Vec2u32 &siz) {

		Vec2f64 scaledSize = siz.cast<Vec2f64>() * info.viewportScale;

		if ((scaledSize > u16_MAX).any())
			oic::System::log()->fatal("Framebuffer::onResize texture limit reached");

		Vec2u16 size = scaledSize.cast<Vec2u16>();

		if (info.size == size && (info.isDynamic || (!info.isDynamic && data->index)))
			return;

		info.size = size;

		if (data->index) {
			glDeleteFramebuffers(1, &data->index);
			data->index = 0;
		}

		if(depth)
			depth->onResize(size.cast<Vec2u32>());

		for (auto *target : targets)
			target->onResize(size.cast<Vec2u32>());

		if (!size.x)
			return;

		glCreateFramebuffers(1, &data->index);
		GLuint fb = data->index;

		glObjectLabel(
			GL_FRAMEBUFFER, fb, GLsizei(getName().size()), getName().c_str()
		);

		if (depth) {

			if(info.keepDepth)
				glNamedFramebufferTexture(fb, GL_DEPTH_ATTACHMENT, depth->getData()->handle, 0);
			else
				glNamedFramebufferRenderbuffer(
					fb,
					FormatHelper::hasStencil(info.depthFormat) ? 
					GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, 
					GL_RENDERBUFFER, depth->getData()->handle
				);
		}

		List<GLenum> drawBuffers(info.colorFormats.size());

		for (usz i = 0, j = targets.size(); i < j; ++i)
			glNamedFramebufferTexture(fb, drawBuffers[i] = GL_COLOR_ATTACHMENT0 + GLenum(i), targets[i]->getData()->handle, 0);

		glNamedFramebufferDrawBuffers(fb, GLsizei(drawBuffers.size()), drawBuffers.data());
		GLenum status = glCheckNamedFramebufferStatus(fb, GL_FRAMEBUFFER);

		if (status != GL_FRAMEBUFFER_COMPLETE)
			oic::System::log()->fatal("Couldn't create framebuffer");

	}

	void Framebuffer::begin() {
		glxBeginRenderPass(
			g.getData()->getContext(), data->index
		);
	}

	void Framebuffer::end() {}

}