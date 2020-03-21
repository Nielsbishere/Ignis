#include "graphics/memory/gl_texture_object.hpp"
#include "graphics/gl_graphics.hpp"
#include "utils/math.hpp"
#include "utils/hash.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {
	
	Texture::Texture(Graphics &g, const String &name, const Info &inf) :
		TextureObject(g, name, inf), info(inf) {

		data = new Data();

		GLint mipCount = inf.mips;
		GLenum textureFormat = glxColorFormat(inf.format);
		GLenum type = glxGpuFormatType(inf.format);
		GLenum format = glxGpuDataFormat(inf.format);

		glCreateTextures(glxTextureType(inf.textureType), 1, &data->handle);
		GLuint handle = data->handle;

		data->textureViews.push_back({ GPUSubresource::TextureRange(0, 0, inf.mips, inf.layers, inf.textureType), handle });
		
		glObjectLabel(GL_TEXTURE, handle, GLsizei(name.size()), name.c_str());

		switch (inf.textureType) {

		case TextureType::TEXTURE_1D:

			glTextureStorage1D(
				handle, mipCount, textureFormat, inf.dimensions.x
			);

			if (inf.initData.size())
				for (u32 i = 0; i < inf.mips; ++i)
					glTextureSubImage1D(
						handle, i, 0, inf.mipSizes[i].x, format, type, inf.initData[i].data()
					);
			
			break;

		case TextureType::TEXTURE_1D_ARRAY:
		case TextureType::TEXTURE_2D: {

				glTextureStorage2D(
					handle, mipCount, textureFormat,
					inf.dimensions.x,
					std::max(inf.dimensions.y, inf.layers)
				);

				if (inf.initData.size())
					for (u32 i = 0; i < inf.mips; ++i)
						glTextureSubImage2D(
							handle, i, 0, 0,
							inf.mipSizes[i].x,
							std::max(inf.mipSizes[i].y, inf.layers),
							format, type,
							inf.initData[i].data()
						);

			}
			break;

		case TextureType::TEXTURE_CUBE:
		case TextureType::TEXTURE_CUBE_ARRAY:
		case TextureType::TEXTURE_2D_ARRAY:
		case TextureType::TEXTURE_3D: {

				glTextureStorage3D(
					handle, mipCount, textureFormat,
					inf.dimensions.x, inf.dimensions.y,
					std::max(inf.dimensions.z, inf.layers)
				);

				if(inf.initData.size())
					for (u32 i = 0; i < inf.mips; ++i)
						glTextureSubImage3D(
							handle, i, 0, 0, 0,
							inf.mipSizes[i].x,
							inf.mipSizes[i].y,
							std::max(inf.mipSizes[i].z, inf.layers),
							format, type, inf.initData[i].data()
						);

			}
			break;

		default:
			oic::System::log()->fatal("TextureType not supported");
		}

		//Create a framebuffer so copy and clear operations can be done for this texture

		if (u8(inf.usage) & u8(GPUMemoryUsage::GPU_WRITE)) {

			const String fbName = NAME(name + " framebuffer");

			data->framebuffer.resize(inf.layers);

			for (u16 i = 0; i < inf.layers; ++i) {

				auto &fb = data->framebuffer[i];

				glCreateFramebuffers(1, &fb);
				glObjectLabel(GL_FRAMEBUFFER, fb, GLsizei(fbName.size()), fbName.c_str());

				GLenum colorAttachment = GL_COLOR_ATTACHMENT0;

				if (inf.layers > 1)
					glNamedFramebufferTextureLayer(fb, colorAttachment, data->handle, 0, i);
				else
					glNamedFramebufferTexture(fb, colorAttachment, data->handle, 0);

				glNamedFramebufferDrawBuffers(fb, 1, &colorAttachment);
				GLenum status = glCheckNamedFramebufferStatus(fb, GL_FRAMEBUFFER);

				if (status != GL_FRAMEBUFFER_COMPLETE)
					oic::System::log()->fatal("Couldn't create GPU write target");

			}
		}

		//Make sure CPU can write into the buffer

		if (u8(inf.usage & GPUMemoryUsage::CPU_WRITE) && inf.initData.empty()) {

			info.initData.resize(inf.mips);

			for (u8 i = 0; i < inf.mips; ++i)
				info.initData[i].resize(info.mipSizes[i].prod<usz>() * inf.layers * FormatHelper::getSizeBytes(inf.format));

		}
	}

	Texture::~Texture() {

		for(auto &views : data->textureViews)
			if (views.second != data->handle) {
				g.getData()->removeTexture(views.second);
				glDeleteTextures(1, &views.second);
			}

		g.getData()->removeTexture(data->handle);
		glDeleteTextures(1, &data->handle);
		destroy(data);
	}

	void Texture::flush(const List<Vec2u8> &ranges) {

		if (!u8(info.usage & GPUMemoryUsage::CPU_WRITE))
			throw std::runtime_error("Flush can only be called on a CPU writable texture");

		GLuint handle = data->handle;
		GLenum type = glxGpuFormatType(info.format);
		GLenum format = glxGpuDataFormat(info.format);

		for (auto& range : ranges)
			for (u8 i = range.x, j = i + range.y; i < j; ++i) 
				switch (info.textureType) {

					case TextureType::TEXTURE_1D:

						glTextureSubImage1D(
							handle, i, 0, 
							info.mipSizes[i].x, 
							format, type,
							info.initData[i].data()
						);

						break;

					case TextureType::TEXTURE_1D_ARRAY:
					case TextureType::TEXTURE_2D:

						glTextureSubImage2D(
							handle, i, 0, 0,
							info.mipSizes[i].x,
							std::max(info.mipSizes[i].y, info.layers),
							format, type,
							info.initData[i].data()
						);

						break;

					case TextureType::TEXTURE_CUBE:
					case TextureType::TEXTURE_CUBE_ARRAY:
					case TextureType::TEXTURE_2D_ARRAY:
					case TextureType::TEXTURE_3D:

						glTextureSubImage3D(
							handle, i, 0, 0, 0,
							info.mipSizes[i].x, info.mipSizes[i].y,
							std::max(info.mipSizes[i].z, info.layers),
							format, type,
							info.initData[i].data()
						);

						break;

				}
	}

}