#include "graphics/memory/gl_texture.hpp"
#include "utils/math.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {
	
	Texture::Texture(Graphics &g, const String &name, const Info &info) :
		GraphicsObject(g, name), info(info) {

		data = new Data();

		GLint mipCount = info.mips;
		GLenum textureFormat = glxColorFormat(info.format);
		GLenum type = glxGpuFormatType(info.format);
		GLenum format = glxGpuDataFormat(info.format);

		Vec3u32 dim = info.dimensions;

		glCreateTextures(glxTextureType(info.textureType), 1, &data->handle);
		GLuint handle = data->handle;

		data->textureViews.push_back({ GPUSubresource::TextureRange(0, 0, info.mips, info.layers, info.textureType), handle });
		
		glObjectLabel(GL_TEXTURE, handle, GLsizei(name.size()), name.c_str());

		switch (info.textureType) {

		case TextureType::TEXTURE_1D:

			glTextureStorage1D(
				handle, mipCount, textureFormat, dim.x
			);

			if (info.initData.size())
				for (u32 i = 0; i < info.mips; ++i) {

					glTextureSubImage1D(
						handle, i, 0, dim.x, format, type, info.initData[i].data()
					);

					dim.x = u32(oic::Math::ceil(dim.x / 2.0));
				}
			
			break;

		case TextureType::TEXTURE_1D_ARRAY:
		case TextureType::TEXTURE_2D: {

				dim.y = oic::Math::max(dim.y, info.layers);
				Vec3f32 div = { 2, info.layers <= 1 ? 2 : 1, 1 };

				glTextureStorage2D(
					handle, mipCount, textureFormat, dim.x, dim.y
				);

				if (info.initData.size())
					for (u32 i = 0; i < info.mips; ++i) {

						glTextureSubImage2D(
							handle, i, 0, 0, dim.x, dim.y, format, type, info.initData[i].data()
						);

						dim = (dim.cast<Vec3f32>() / div).ceil().cast<Vec3u32>();
					}

			}
			break;

		case TextureType::TEXTURE_CUBE:
		case TextureType::TEXTURE_CUBE_ARRAY:
		case TextureType::TEXTURE_2D_ARRAY:
		case TextureType::TEXTURE_3D: {

				dim.z = oic::Math::max(dim.z, info.layers);
				Vec3f32 div(2, 2, info.layers <= 1 ? 2 : 1);

				glTextureStorage3D(
					handle, mipCount, textureFormat, dim.x, dim.y, dim.z
				);

				if(info.initData.size())
					for (u32 i = 0; i < info.mips; ++i) {

						glTextureSubImage3D(
							handle, i, 0, 0, 0, dim.x, dim.y, dim.z,
							format, type, info.initData[i].data()
						);

						dim = (dim.cast<Vec3f32>() / div).ceil().cast<Vec3u32>();
					}

			}
			break;

		default:
			oic::System::log()->fatal("TextureType not supported");
		}
	}

	Texture::~Texture() {

		for(auto &views : data->textureViews)
			if(views.second != data->handle)
				glDeleteTextures(1, &data->handle);

		glDeleteTextures(1, &data->handle);
		delete data;
	}

}