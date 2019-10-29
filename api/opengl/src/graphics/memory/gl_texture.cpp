#include "graphics/memory/gl_texture.hpp"
#include "utils/math.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {
	
	Texture::Texture(Graphics &g, const String &name, const Info &info) :
		GPUResource(g, name), info(info) {

		data = new Data();

		GLint mipCount = u8(info.mips) & u8(TextureMip::PROPERTY_MIP_COUNT);
		GLenum textureFormat = glxColorFormat(info.format);
		GLenum type = glxGpuFormatType(info.format);
		GLenum format = glxGpuDataFormat(info.format);

		u32 x = info.dimensions[0];

		glCreateTextures(glxTextureType(info.textureType), 1, &data->handle);
		GLuint handle = data->handle;
		
		glObjectLabel(GL_TEXTURE, handle, GLsizei(name.size()), name.c_str());

		switch (info.textureType) {

		case TextureType::TEXTURE_1D:

			glTextureStorage1D(
				handle, mipCount, textureFormat, info.dimensions[0]
			);

			if(info.initData.size())
				glTextureSubImage1D(
					handle, 0, 0, x, format, type, info.initData.data()
				);
			break;

		case TextureType::TEXTURE_1D_ARRAY:
		case TextureType::TEXTURE_2D: {

				u32 y = oic::Math::max(info.dimensions[1], info.layers);

				glTextureStorage2D(
					handle, mipCount, textureFormat, x, y
				);

				if(info.initData.size())
					glTextureSubImage2D(
						handle, 0, 0, 0, x, y, format, type, info.initData.data()
					);

			}
			break;

		case TextureType::TEXTURE_2D_ARRAY:
		case TextureType::TEXTURE_3D: {

				u32 y = info.dimensions[1];
				u32 z = oic::Math::max(info.dimensions[2], info.layers);

				glTextureStorage3D(
					handle, mipCount, textureFormat, x, y, z
				);

				if(info.initData.size())
					glTextureSubImage3D(
						handle, 0, 0, 0, 0, x, y, z, format, type, info.initData.data()
					);

			}
			break;

		default:
			oic::System::log()->fatal("TextureType not supported");
		}
	}

	Texture::~Texture() {
		glDeleteTextures(1, &data->handle);
		delete data;
	}

}