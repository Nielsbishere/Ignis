#include "graphics/memory/depth_texture.hpp"
#include "graphics/memory/gl_texture_object.hpp"
#include "utils/hash.hpp"

namespace ignis {

	DepthTexture::DepthTexture(Graphics &g, const String &name, const Info &info) :
		TextureObject(g, name, info), format(info.format), storeData(info.storeData)
	{
		data = new Data();

		oicAssert(
			"Invalid texture type for DepthTexture",
			info.textureType == TextureType::TEXTURE_MS || info.textureType == TextureType::TEXTURE_2D ||
			info.textureType == TextureType::TEXTURE_MS_ARRAY || info.textureType == TextureType::TEXTURE_2D_ARRAY
		);
	}

	DepthTexture::~DepthTexture() {
		onResize({});
		destroy(data);
	}

	void DepthTexture::onResize(const Vec2u32 &size) {

		for(auto &views : data->textureViews)
			if (views.second != data->handle) {
				glDeleteTextures(1, &views.second);
				views.second = 0;
			}

		data->textureViews.clear();

		if (data->handle) {

			if (storeData)
				glDeleteTextures(1, &data->handle);
			else
				glDeleteRenderbuffers(1, &data->handle);

			data->handle = 0;
		}

		if (!size.all())
			return;

		if (size == info.dimensions.cast<Vec2u32>())
			return;

		info.dimensions.x = u16(size.x);
		info.dimensions.y = u16(size.y);

		if (format == DepthFormat::AUTO_DEPTH)
			format = DepthFormat::D32;

		else if(format == DepthFormat::AUTO_DEPTH_STENCIL)
			format = DepthFormat::D24_S8;

		//TODO: Stencil buffers

		if (storeData) {

			glCreateTextures(
				glxTextureType(info.textureType), 1, &data->handle
			);

			GLuint handle = data->handle;

			String hashed = NAME(getName() + " depth texture");
			glObjectLabel(
				GL_TEXTURE, handle, GLsizei(hashed.size()), hashed.c_str()
			);

			if (info.layers > 1) {

				if (info.samples > 1)
					glTextureStorage3DMultisample(
						handle, GLsizei(info.samples),
						glxDepthFormat(format),
						size.x, size.y, info.layers,
						info.useFixedSampleLocations

					);
				else
					glTextureStorage3D(
						handle, info.mips,
						glxDepthFormat(format),
						size.x, size.y, info.layers
					);

			} else if (info.samples > 1)
				glTextureStorage2DMultisample(
					handle, GLsizei(info.samples),
					glxDepthFormat(format),
					size.x, size.y,
					info.useFixedSampleLocations
				);
			else
				glTextureStorage2D(
					handle, info.mips,
					glxDepthFormat(format),
					size.x, size.y
				);

		} else {

			glCreateRenderbuffers(1, &data->handle);
			GLuint handle = data->handle;

			String hashed = NAME(getName() + " depth buffer");
			glObjectLabel(
				GL_RENDERBUFFER, handle, GLsizei(hashed.size()), hashed.c_str()
			);

			glNamedRenderbufferStorageMultisample(
				handle, GLsizei(info.samples),
				glxDepthFormat(format), size.x, size.y
			);
		}
	}
}