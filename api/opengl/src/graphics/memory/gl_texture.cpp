#include "graphics/memory/gl_texture_object.hpp"
#include "graphics/gl_graphics.hpp"
#include "utils/math.hpp"
#include "utils/hash.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {
	
	Texture::Texture(Graphics &g, const String &name, const Info &inf) :
		TextureObject(g, name, inf, GPUObjectType::TEXTURE), info(inf)
	{

		data = new Data();

		GLint mipCount = inf.mips;
		GLenum textureFormat = glxColorFormat(inf.format);

		glCreateTextures(glxTextureType(inf.textureType), 1, &data->handle);
		GLuint handle = data->handle;

		data->textureViews.push_back({ GPUSubresource::TextureRange(0, 0, inf.mips, inf.layers, inf.textureType), handle });
		
		glObjectLabel(GL_TEXTURE, handle, GLsizei(name.size()), name.c_str());

		switch (inf.textureType) {

		case TextureType::TEXTURE_1D:

			glTextureStorage1D(
				handle, mipCount, textureFormat, inf.dimensions.x
			);
			
			break;

		case TextureType::TEXTURE_1D_ARRAY:
		case TextureType::TEXTURE_2D: {

				glTextureStorage2D(
					handle, mipCount, textureFormat,
					inf.dimensions.x,
					std::max(inf.dimensions.y, inf.layers)
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

			}
			break;

		default:
			oic::System::log()->fatal("TextureType not supported");
		}

		//Create a framebuffer so copy and clear operations can be done for this texture

		if (u8(inf.usage) & u8(GPUMemoryUsage::GPU_WRITE)) {

			const String fbName = NAME(name + " framebuffer");

			data->framebuffer.resize(size_t(inf.layers) * inf.mips);

			for (u16 i = 0; i < inf.layers * inf.mips; ++i) {

				auto &fb = data->framebuffer[i];

				glCreateFramebuffers(1, &fb);
				glObjectLabel(GL_FRAMEBUFFER, fb, GLsizei(fbName.size()), fbName.c_str());

				GLenum colorAttachment = GL_COLOR_ATTACHMENT0;

				if (inf.layers > 1)
					glNamedFramebufferTextureLayer(fb, colorAttachment, data->handle, i % inf.mips, i / inf.mips);
				else
					glNamedFramebufferTexture(fb, colorAttachment, data->handle, i);

				glNamedFramebufferDrawBuffers(fb, 1, &colorAttachment);
				GLenum status = glCheckNamedFramebufferStatus(fb, GL_FRAMEBUFFER);

				if (status != GL_FRAMEBUFFER_COMPLETE)
					oic::System::log()->fatal("Couldn't create GPU write target");

			}
		}

		//Make sure CPU can write into the buffer

		if (inf.initData.size() < inf.mips) {

			u8 start = u8(inf.initData.size());
			info.initData.resize(inf.mips);

			for (u8 i = start; i < inf.mips; ++i)
				info.initData[i].resize(info.mipSizes[i].prod<usz>() * inf.layers * FormatHelper::getSizeBytes(inf.format));

		}
	}

	Texture::~Texture() {

		for(auto &views : data->textureViews)
			if (views.second != data->handle)
				glDeleteTextures(1, &views.second);

		glDeleteTextures(1, &data->handle);
		destroy(data);
	}

	void Texture::flush(CommandList::Data*, UploadBuffer *uploadBuffer, u8 mip, u8 mipCount) {

		if (info.initData.size() < info.mips) {
			oic::System::log()->error("Texture didn't have any backing CPU data");
			return;
		}

		if (!HasFlags(info.usage, GPUMemoryUsage::SHARED) && !uploadBuffer) {
			oic::System::log()->error("Even though OpenGL handles UploadBuffers implictly, for non shared memory one is required by the ignis spec");
			return;
		}

		GLuint handle = data->handle;
		GLenum type = glxGpuFormatType(info.format);
		GLenum format = glxGpuDataFormat(info.format);

		for (u8 i = mip, j = mip + mipCount; i < j; ++i)

			if (i >= info.mips) {
				oic::System::log()->error("Flush texture mip out of bounds");
				break;
			}

			else switch (info.textureType) {

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

		//First flush is only for submitting the initial texture. Then the data is removed
		if (!HasFlags(info.usage, GPUMemoryUsage::CPU_ACCESS)) {
			info.initData.clear();
			return;
		}
	}

}