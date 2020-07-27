#include "graphics/memory/render_texture.hpp"
#include "graphics/memory/gl_texture_object.hpp"
#include "utils/hash.hpp"

namespace ignis {

	RenderTexture::RenderTexture(Graphics &g, const String &name, const Info &info) :
		TextureObject(g, name, info, GPUObjectType::RENDER_TEXTURE)
	{
		data = new Data();

		oicAssert(
			"Invalid texture type for RenderTexture",
			info.textureType == TextureType::TEXTURE_MS || info.textureType == TextureType::TEXTURE_2D ||
			info.textureType == TextureType::TEXTURE_MS_ARRAY || info.textureType == TextureType::TEXTURE_2D_ARRAY
		);
	}

	RenderTexture::~RenderTexture() {
		onResize({});
		destroy(data);
	}

	void RenderTexture::onResize(const Vec2u32 &size) {

		for(auto &views : data->textureViews)
			if (views.second != data->handle) {
				glDeleteTextures(1, &views.second);
				views.second = 0;
			}

		data->textureViews.clear();

		if (data->handle) {
			glDeleteTextures(1, &data->handle);
			data->handle = 0;
		}

		if (!size.all())
			return;

		if (size == info.dimensions.cast<Vec2u32>())
			return;

		info.dimensions.x = u16(size.x);
		info.dimensions.y = u16(size.y);
		info.dimensions.z = 1;

		info.mips = 1;
		info.mipSizes = { info.dimensions };

		glCreateTextures(
			glxTextureType(info.textureType), 
			1, &data->handle
		);

		GLuint tex = data->handle;
		if (info.layers > 1) {

			if(info.samples > 1)
				glTextureStorage3DMultisample(
					tex, GLsizei(info.samples),
					glxColorFormat(info.format),
					size.x, size.y, info.layers,
					info.useFixedSampleLocations
				);
			else
				glTextureStorage3D(
					tex, info.mips,
					glxColorFormat(info.format),
					size.x, size.y, info.layers
				);
		}
		else if(info.samples > 1)
			glTextureStorage2DMultisample(
				tex, GLsizei(info.samples),
				glxColorFormat(info.format),
				size.x, size.y,
				info.useFixedSampleLocations
			);
		else
			glTextureStorage2D(
				tex, info.mips,
				glxColorFormat(info.format),
				size.x, size.y
			);

		const String &hashed = getName();
		glObjectLabel(GL_TEXTURE, tex, GLsizei(hashed.size()), hashed.c_str());
	}
}