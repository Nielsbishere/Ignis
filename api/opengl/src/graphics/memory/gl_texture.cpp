#include "graphics/memory/gl_texture.hpp"
#include "utils/math.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {
	
	Texture::Texture(Graphics &g, const String &name, const Info &info) :
		GPUResource(g, name), info(info) {

		data = new Data();

		GLint mipCount = info.mips;
		GLenum textureFormat = glxColorFormat(info.format);
		GLenum type = glxGpuFormatType(info.format);
		GLenum format = glxGpuDataFormat(info.format);

		u32 x = info.dimensions[0];

		glCreateTextures(glxTextureType(info.textureType), 1, &data->handle);
		GLuint handle = data->handle;

		data->textureViews.push_back({ GPUSubresource::TextureRange(0, 0, info.mips, info.layers), handle });
		
		glObjectLabel(GL_TEXTURE, handle, GLsizei(name.size()), name.c_str());

		switch (info.textureType) {

		case TextureType::TEXTURE_1D:

			glTextureStorage1D(
				handle, mipCount, textureFormat, info.dimensions[0]
			);

			if (info.initData.size())
				for (u32 i = 0; i < info.mips; ++i) {

					glTextureSubImage1D(
						handle, i, 0, x, format, type, info.initData[i].data()
					);

					x = u32(oic::Math::ceil(x / 2.0));
				}
			
			break;

		case TextureType::TEXTURE_1D_ARRAY:
		case TextureType::TEXTURE_2D: {

				u32 y = oic::Math::max(info.dimensions[1], info.layers);
				f64 yDiv = info.layers <= 1 ? 2 : 1;

				glTextureStorage2D(
					handle, mipCount, textureFormat, x, y
				);

				if (info.initData.size())
					for (u32 i = 0; i < info.mips; ++i) {

						glTextureSubImage2D(
							handle, i, 0, 0, x, y, format, type, info.initData[i].data()
						);

						x = u32(oic::Math::ceil(x / 2.0));
						y = u32(oic::Math::ceil(y / yDiv));
					}

			}
			break;

		case TextureType::TEXTURE_2D_ARRAY:
		case TextureType::TEXTURE_3D: {

				u32 y = info.dimensions[1];
				u32 z = oic::Math::max(info.dimensions[2], info.layers);
				f64 zDiv = info.layers <= 1 ? 2 : 1;

				glTextureStorage3D(
					handle, mipCount, textureFormat, x, y, z
				);

				if(info.initData.size())
					for (u32 i = 0; i < info.mips; ++i) {

						glTextureSubImage3D(
							handle, i, 0, 0, 0, x, y, z, format, type, info.initData[i].data()
						);

						x = u32(oic::Math::ceil(x / 2.0));
						y = u32(oic::Math::ceil(y / 2.0));
						z = u32(oic::Math::ceil(z / zDiv));
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