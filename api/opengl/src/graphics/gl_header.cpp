#include "graphics/format.hpp"
#include "system/log.hpp"
#include "system/system.hpp"
#include "graphics/gl_graphics.hpp"

HashMap<String, void**> ignis::glFunctionNames = HashMap<String, void**>();

template<typename T>
static T appendGlFunc(const String &s, T &t) {
	ignis::glFunctionNames[s] = (void**)&t;
	return nullptr;
}

#define GL_FUNC(x, y) PFN##y##PROC x = appendGlFunc(#x, x)

#include "graphics/gl_functions.hpp"

using namespace ignis;

void ::glBeginRenderPass(
	ignis::Graphics::Data &gdata, const Vec4u &xywh, const Vec2u &size, GLuint framebuffer
) {

	if (gdata.framebuffer != framebuffer)
		glBindFramebuffer(GL_FRAMEBUFFER, gdata.framebuffer = framebuffer);

	Vec4u sc = xywh;

	if (!sc[2])
		sc[2] = size[0] - sc[0];

	if (!sc[3])
		sc[3] = size[1] - sc[1];

	Vec4u vp = { 0, 0, size[0] - sc[0], size[1] - sc[1] };

	if (gdata.viewport != vp) {
		gdata.viewport = vp;
		glViewport(vp[0], vp[1], vp[2], vp[3]);
	}

	if (sc == vp) {
		if (gdata.scissorEnable) {
			glDisable(GL_SCISSOR_TEST);
			gdata.scissorEnable = false;
		}
	} else if (!gdata.scissorEnable) {
		glEnable(GL_SCISSOR_TEST);
		gdata.scissorEnable = true;
	}

	if (gdata.scissor != sc) {
		gdata.scissor = sc;
		glScissor(sc[0], sc[1], sc[2], sc[3]);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

GLenum glDepthFormat(ignis::DepthFormat format) {

	switch (format) {

		case DepthFormat::D16: return GL_DEPTH_COMPONENT16;
		case DepthFormat::D32: return GL_DEPTH_COMPONENT32;

		case DepthFormat::D24_S8:
		case DepthFormat::D24: 
			return GL_DEPTH_COMPONENT24;

		case DepthFormat::D32F:
		case DepthFormat::D32F_S8:
			return GL_DEPTH_COMPONENT32F;

		default:
			oic::System::log()->fatal("Invalid depth format");
			return GL_DEPTH_COMPONENT32F;
	}

}

GLenum glColorFormat(ignis::GPUFormat format){

	switch (format) {

		case GPUFormat::R8:		return GL_R8;
		case GPUFormat::RG8:	return GL_RG8;
		case GPUFormat::BGR8:
		case GPUFormat::RGB8:	return GL_RGB8;
		case GPUFormat::BGRA8:
		case GPUFormat::RGBA8:	return GL_RGBA8;

		case GPUFormat::R16:	return GL_R16;
		case GPUFormat::RG16:	return GL_RG16;
		case GPUFormat::RGB16:	return GL_RGB16;
		case GPUFormat::RGBA16:	return GL_RGBA16;

		case GPUFormat::R8s:	return GL_R8_SNORM;
		case GPUFormat::RG8s:	return GL_RG8_SNORM;
		case GPUFormat::BGR8s:
		case GPUFormat::RGB8s:	return GL_RGB8_SNORM;
		case GPUFormat::BGRA8s:
		case GPUFormat::RGBA8s:	return GL_RGBA8_SNORM;

		case GPUFormat::R16s:	return GL_R16_SNORM;
		case GPUFormat::RG16s:	return GL_RG16_SNORM;
		case GPUFormat::RGB16s:	return GL_RGB16_SNORM;
		case GPUFormat::RGBA16s:return GL_RGBA16_SNORM;

		case GPUFormat::R8u:	return GL_R8UI;
		case GPUFormat::RG8u:	return GL_RG8UI;
		case GPUFormat::BGR8u:
		case GPUFormat::RGB8u:	return GL_RGB8UI;
		case GPUFormat::BGRA8u:
		case GPUFormat::RGBA8u:	return GL_RGBA8UI;

		case GPUFormat::R16u:	return GL_R16UI;
		case GPUFormat::RG16u:	return GL_RG16UI;
		case GPUFormat::RGB16u:	return GL_RGB16UI;
		case GPUFormat::RGBA16u:return GL_RGBA16UI;

		case GPUFormat::R32u:	return GL_R32UI;
		case GPUFormat::RG32u:	return GL_RG32UI;
		case GPUFormat::RGB32u:	return GL_RGB32UI;
		case GPUFormat::RGBA32u:return GL_RGBA32UI;

		case GPUFormat::R8i:	return GL_R8I;
		case GPUFormat::RG8i:	return GL_RG8I;
		case GPUFormat::BGR8i:
		case GPUFormat::RGB8i:	return GL_RGB8I;
		case GPUFormat::BGRA8i:
		case GPUFormat::RGBA8i:	return GL_RGBA8I;

		case GPUFormat::R16i:	return GL_R16I;
		case GPUFormat::RG16i:	return GL_RG16I;
		case GPUFormat::RGB16i:	return GL_RGB16I;
		case GPUFormat::RGBA16i:return GL_RGBA16I;

		case GPUFormat::R32i:	return GL_R32I;
		case GPUFormat::RG32i:	return GL_RG32I;
		case GPUFormat::RGB32i:	return GL_RGB32I;
		case GPUFormat::RGBA32i:return GL_RGBA32I;

		case GPUFormat::R16f:	return GL_R16F;
		case GPUFormat::RG16f:	return GL_RG16F;
		case GPUFormat::RGB16f:	return GL_RGB16F;
		case GPUFormat::RGBA16f:return GL_RGBA16F;

		case GPUFormat::R32f:	return GL_R32F;
		case GPUFormat::RG32f:	return GL_RG32F;
		case GPUFormat::RGB32f:	return GL_RGB32F;
		case GPUFormat::RGBA32f:return GL_RGBA32F;

		case GPUFormat::sBGR8:
		case GPUFormat::sRGB8:	return GL_SRGB8;

		case GPUFormat::sBGRA8:
		case GPUFormat::sRGBA8:	return GL_SRGB8_ALPHA8;

		case GPUFormat::R64f: case GPUFormat::R64u: case GPUFormat::R64i:
		case GPUFormat::RG64f: case GPUFormat::RG64u: case GPUFormat::RG64i:
		case GPUFormat::RGB64f: case GPUFormat::RGB64u: case GPUFormat::RGB64i:
		case GPUFormat::RGBA64f: case GPUFormat::RGBA64u: case GPUFormat::RGBA64i:
			oic::System::log()->fatal("OpenGL doesn't support 64-bit buffers");

		default:
			oic::System::log()->fatal("Invalid depth format");
			return GL_RGBA8;

	}

}
