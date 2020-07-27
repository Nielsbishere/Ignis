#include "graphics/memory/gl_texture_object.hpp"
#include "graphics/memory/depth_texture.hpp"
#include "graphics/gl_graphics.hpp"
#include "utils/math.hpp"
#include "utils/hash.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {
	
	Texture::Texture(Graphics &g, const String &name, const Info &inf) :
		TextureObject(g, name, inf, GPUObjectType::TEXTURE), info(inf)
	{
		for(u8 i{}; i < inf.mips; ++i) {

			auto mipSize = inf.mipSizes[i];

			auto &layer = mipSize.arr[getDimensionLayerId()];
			layer = std::max(layer, inf.layers);

			info.pending.push_back(
				TextureRange { {}, mipSize, i }
			);
		}

		data = new Data();

		GLint mipCount = inf.mips;
		GLenum textureFormat = glxColorFormat(inf.format);

		glCreateTextures(glxTextureType(inf.textureType), 1, &data->handle);
		GLuint handle = data->handle;

		data->textureViews.push_back({ GPUSubresource::TextureRange(0, 0, inf.mips, inf.layers, inf.textureType), handle });
		
		glObjectLabel(GL_TEXTURE, handle, GLsizei(name.size()), name.c_str());

		switch (inf.textureType) {

			case TextureType::TEXTURE_1D:
				glTextureStorage1D(handle, mipCount, textureFormat, inf.dimensions.x);
				break;

			case TextureType::TEXTURE_1D_ARRAY:
			case TextureType::TEXTURE_2D:

				glTextureStorage2D(
					handle, mipCount, textureFormat,
					inf.dimensions.x,
					std::max(inf.dimensions.y, inf.layers)
				);

				break;

			case TextureType::TEXTURE_CUBE:
			case TextureType::TEXTURE_CUBE_ARRAY:
			case TextureType::TEXTURE_2D_ARRAY:
			case TextureType::TEXTURE_3D:

				glTextureStorage3D(
					handle, mipCount, textureFormat,
					inf.dimensions.x, inf.dimensions.y,
					std::max(inf.dimensions.z, inf.layers)
				);

				break;

			default:
				oic::System::log()->fatal("TextureType not supported");
		}

		//Create a framebuffer so copy and clear operations can be done for this texture

		if (HasFlags(inf.usage, GPUMemoryUsage::GPU_WRITE)) {

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

		if(!HasFlags(inf.usage, GPUMemoryUsage::NO_CPU_MEMORY) && inf.initData.size() < inf.mips) {

			u8 start = u8(inf.initData.size());
			info.initData.resize(inf.mips);

			for (u8 i = start; i < inf.mips; ++i)
				info.initData[i].resize(info.mipSizes[i].prod<usz>() * inf.layers * FormatHelper::getSizeBytes(inf.format));

		}
	}

	Texture::~Texture() {

		if (!data) return;

		if (HasFlags(info.usage, GPUMemoryUsage::GPU_WRITE))
			for (u16 i = 0; i < info.layers * info.mips; ++i)
				glDeleteFramebuffers(1, &data->framebuffer[i]);

		for(auto &views : data->textureViews)
			if (views.second != data->handle)
				glDeleteTextures(1, &views.second);

		glDeleteTextures(1, &data->handle);
		destroy(data);
	}

	Pair<u64, u64> Texture::prepare(CommandList::Data*, UploadBuffer*) {
		return { 0, u64_MAX };		//Since textures are transfered from the CPU, we don't need an allocation
	}

	void Texture::flush(CommandList::Data*, UploadBuffer *uploadBuffer, const Pair<u64, u64>&) {

		if (info.pending.empty())
			return;

		if (!HasFlags(info.usage, GPUMemoryUsage::SHARED) && !uploadBuffer) {
			oic::System::log()->error("Even though OpenGL handles UploadBuffers implictly, for non shared memory one is required by the ignis spec");
			return;
		}

		GLuint handle = data->handle;
		GLenum type = glxGpuFormatType(info.format);
		GLenum format = glxGpuDataFormat(info.format);

		for (auto &pending : info.pending) {

			if (info.initData.size() <= pending.mip) {
				oic::System::log()->error("Texture didn't have any backing CPU data");
				continue;
			}

			Vec3usz dimensions = getDimensions(pending.mip).cast<Vec3usz>();

			auto *src = info.initData[pending.mip].data();
			usz stride = FormatHelper::getSizeBytes(info.format);

			Buffer copyData;

			bool needsCopy;

			switch (info.textureType) {

				case TextureType::TEXTURE_1D:
					needsCopy = false;
					break;

				case TextureType::TEXTURE_1D_ARRAY:
				case TextureType::TEXTURE_2D:
					needsCopy = pending.size.x != dimensions.x;
					break;

				default:
					needsCopy = pending.size.x != dimensions.x || pending.size.y != dimensions.y;
			}

			//We need to copy to an intermediate CPU buffer

			if (needsCopy) {

				copyData.resize(stride * pending.size.prod<usz>());

				const usz dstRow = pending.size.x * stride;
				const usz dstSlice = pending.size.y * dstRow;

				const usz srcRow = dimensions.x * stride;
				const usz srcSlice = dimensions.y * srcRow;
				
				//Copy each line

				if(
					info.textureType == TextureType::TEXTURE_1D_ARRAY || 
					info.textureType == TextureType::TEXTURE_2D
				)
					for(

						usz i = 0,

						srcOff = (pending.start.x + pending.start.y * dimensions.x) * stride,
						dstOff{}; 

						i < pending.size.y; 

						++i,

						srcOff += srcRow,
						dstOff += dstRow
					)
						std::memcpy(copyData.data() + dstOff, src + srcOff, dstRow);

				else if(pending.size.x == dimensions.x)
					for (

						usz i{},

						srcOff = stride * (pending.start.x + (pending.start.y + pending.start.z * dimensions.y) * dimensions.x),
						dstOff{}; 

						i < pending.size.z; 

						++i,

						dstOff += dstSlice
					)
						std::memcpy(copyData.data() + dstOff, src + srcOff + srcSlice * i, dstSlice);

				//Copy each line from a 3D texture

				else for(

					usz i{}, j = pending.size.z * usz(pending.size.y),
					y{}, z{},

					srcOff = stride * (pending.start.x + (pending.start.y + pending.start.z * dimensions.y) * dimensions.x),
					dstOff{}; 

					i < j; 

					++i,

					y = i % pending.size.y,
					z = i / pending.size.y,

					dstOff += dstRow
				)
					std::memcpy(copyData.data() + dstOff, src + srcOff + srcRow * y + srcSlice * z, dstRow);

				src = copyData.data();
			}

			//No need to copy, we just need to offset from our CPU data

			else
				src += stride * (pending.start.x + dimensions.x * (pending.start.y + dimensions.y * pending.start.z));

			//Copy to GPU

			switch (info.textureType) {

				case TextureType::TEXTURE_1D:

					glTextureSubImage1D(

						handle,

						pending.mip,

						pending.start.x,
						pending.size.x, 

						format,
						type,
						src
					);

					break;

				case TextureType::TEXTURE_1D_ARRAY:
				case TextureType::TEXTURE_2D:

					glTextureSubImage2D(

						handle,
						
						pending.mip,

						pending.start.x,
						pending.start.y,

						pending.size.x,
						pending.size.y,

						format,
						type,
						src
					);

					break;

				default:

					glTextureSubImage3D(

						handle,
						
						pending.mip,

						pending.start.x,
						pending.start.y,
						pending.start.z,

						pending.size.x,
						pending.size.y,
						pending.size.z,

						format,
						type,
						src
					);
			}
		}

		info.pending.clear();

		//First flush is only for submitting the initial texture. Then the data is removed
		if (!HasFlags(info.usage, GPUMemoryUsage::CPU_WRITE)) {
			info.initData.clear();
			return;
		}
	}

}