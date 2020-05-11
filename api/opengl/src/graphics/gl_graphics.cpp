#include "utils/thread.hpp"
#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"
#include "graphics/gl_graphics.hpp"
#include "graphics/gl_context.hpp"
#include "graphics/memory/gl_framebuffer.hpp"
#include "graphics/memory/gl_texture_object.hpp"
#include "graphics/memory/swapchain.hpp"
#include "graphics/shader/descriptors.hpp"
#include "system/system.hpp"

namespace ignis {

	Graphics::~Graphics() {
		release();
		destroy(data);
	}

	Graphics::Graphics(
		const String &applicationName,
		const u32 applicationVersion,
		const String &engineName,
		const u32 engineVersion
	) throw() :
		appName(applicationName), appVersion(applicationVersion),
		engineName(engineName), engineVersion(engineVersion) {
		data = new Graphics::Data();
		init();
	}

	GraphicsApi Graphics::getCurrentApi() const {
		return GraphicsApi::OPENGL;
	}

	CommandAvailability Graphics::getCommandAvailability(CommandOp op) {

		if (op >> CMD_PROPERTY_TECHNIQUE_SHIFT)
			return CommandAvailability::UNSUPPORTED;

		return CommandAvailability::SUPPORTED;
	}

	void Graphics::execute(const List<CommandList *> &commands) {

		oicAssert("Graphics::execute can't be ran on a suspended graphics thread", enabledThreads[oic::Thread::getCurrentId()].enabled);

		//TODO: lock mutex

		//Updates VAOs and FBOs that have been added/released
		data->updateContext(*this);

		for (CommandList *cl : commands)
			cl->execute();

		//Make sure that all immediate handles are converted to frame independent
		data->storeContext();

		//TODO: unlock mutex
	}

	//Present framebuffer to swapchain

	void Graphics::present(
		Framebuffer *intermediate, Swapchain *swapchain,
		const List<CommandList *> &commands
	) {

		if (!swapchain)
			oic::System::log()->fatal("Couldn't present; invalid intermediate or swapchain");

		if (!intermediate)
			oic::System::log()->warn("Presenting without an intermediate is valid but won't provide any results to the swapchain");

		if (intermediate && intermediate->getInfo().size != swapchain->getInfo().size)
			oic::System::log()->fatal("Couldn't present; swapchain and intermediate aren't same size");

		GLContext &ctx = data->getContext();

		swapchain->bind();

		execute(commands);

		//Copy intermediate to backbuffer
		if (intermediate) {

			Vec2u16 size = intermediate->getInfo().size;

			oicAssert("Framebuffer should have 1 render texture to copy", intermediate->size());
			oicAssert("Framebuffer should have a proper resolution before blit", size.all());

			//Bind backbuffer

			//TODO: Fix this! it should use a shader!

			glxSetViewportAndScissor(ctx, swapchain->getInfo().size.cast<Vec2u32>(), {});

			ctx.bound[GL_READ_FRAMEBUFFER] = intermediate->getId();
			ctx.bound[GL_DRAW_FRAMEBUFFER] = {};

			glBlitNamedFramebuffer(
				intermediate->getData()->index,
				0,
				0, 0, size.x, size.y,
				0, size.y, size.x, 0,
				GL_COLOR_BUFFER_BIT, GL_LINEAR
			);
		}

		swapchain->present();
		++ctx.frameId;
	}

	//Present image to swapchain

	void Graphics::present(
		Texture *intermediate, u16 slice, u16 mip,
		Swapchain *swapchain,
		const List<CommandList*> &commands
	) {

		if (!swapchain)
			oic::System::log()->fatal("Couldn't present; invalid intermediate or swapchain");

		if(!intermediate)
			oic::System::log()->warn("Presenting without an intermediate is valid but won't provide any results to the swapchain");

		const TextureType tt = intermediate->getInfo().textureType;

		if(
			tt == TextureType::TEXTURE_MS || tt == TextureType::TEXTURE_MS_ARRAY || 
			tt == TextureType::TEXTURE_1D || tt == TextureType::TEXTURE_1D_ARRAY
		)
			oic::System::log()->fatal("Couldn't present; intermediate texture has to be 2D/2D[], 3D/3D[] or Cube/Cube[]");

		auto size = intermediate->getInfo().dimensions.cast<Vec2u16>();
		size = size.min(swapchain->getInfo().size.cast<Vec2u16>());			//Copy the smallest region to the swapchain; the intermediate may be bigger

		if(slice >= intermediate->getInfo().layers)
			oic::System::log()->fatal("Couldn't present; array index out of bounds");

		if(slice >= intermediate->getInfo().mips)
			oic::System::log()->fatal("Couldn't present; mip index out of bounds");

		GLContext &ctx = data->getContext();

		swapchain->bind();

		execute(commands);

		//Copy intermediate to backbuffer
		if (intermediate) {

			glxSetViewportAndScissor(ctx, swapchain->getInfo().size.cast<Vec2u32>(), {});

			ctx.bound[GL_READ_FRAMEBUFFER] = intermediate->getId();
			ctx.bound[GL_DRAW_FRAMEBUFFER] = {};

			glBlitNamedFramebuffer(
				intermediate->getData()->framebuffer[size_t(slice) * intermediate->getInfo().mips + mip],
				0,
				0, 0, size.x, size.y,
				0, 0, size.x, size.y,
				GL_COLOR_BUFFER_BIT, GL_LINEAR
			);
		}

		swapchain->present();
		++ctx.frameId;
	}

	void Graphics::Data::updateContext(Graphics &g) {

		GLContext &ctx = getContext();

		//Acquire resources from id (since they might be destroyed)

		ctx.framebuffer = ctx.framebufferId.get<Framebuffer>();
		ctx.primitiveBuffer = ctx.primitiveBufferId.get<PrimitiveBuffer>();
		ctx.pipeline = ctx.pipelineId.get<Pipeline>();
		ctx.descriptors = ctx.descriptorsId.get<Descriptors>();

		//Unbind if the resource stopped existing

		for(auto &bound : ctx.bound)
			if (bound.second.vanished()) {

				switch(bound.first) {
				
					case GL_DRAW_FRAMEBUFFER:
					case GL_READ_FRAMEBUFFER:
						glBindFramebuffer(bound.first, 0);
				
				}

				bound.second = {};
			}

		for(auto &bound : ctx.boundByBaseId)
			if (bound.second.id.vanished()) {

				GLenum en = bound.first & u32_MAX;
				u32 i = bound.first >> 32;

				switch(en) {
				
					case GL_SAMPLER:
						glBindSampler(i, 0);
						break;

					case GL_TEXTURE:
						glBindTexture(i, 0);
						break;

					case GL_IMAGE_2D:
						glBindImageTexture(i, 0, 0, false, 0, GL_WRITE_ONLY, GL_RGBA8);
						break;

					case GL_UNIFORM_BUFFER:
					case GL_SHADER_STORAGE_BUFFER:
						glBindBufferRange(en, i, 0, 0, 0);
				
				}

				bound.second = {};
			}

		//Clean up left over VAOs

		auto &deleted = g.getDeletedObjects();

		for (const auto &del : deleted) {

			if (del.type == GPUObjectType::PRIMITIVE_BUFFER) {
				glDeleteVertexArrays(1, &ctx.vaos[del]);
				ctx.vaos.erase(del);
			}
		}

		deleted.clear();

	}

	void Graphics::Data::storeContext() {

		GLContext &ctx = getContext();

		//Store frame independent ids

		ctx.framebufferId = ctx.framebuffer->getId();
		ctx.primitiveBufferId = ctx.primitiveBuffer->getId();
		ctx.pipelineId = ctx.pipeline->getId();
		ctx.descriptorsId = ctx.descriptors->getId();

	}

	void Graphics::Data::destroyContext() {

		GLContext &context = getContext();
		
		for(auto &vao : context.vaos)
			glDeleteVertexArrays(1, &vao.second);

		contexts.erase(oic::Thread::getCurrentId());
	}

	GLContext &Graphics::Data::getContext() {

		auto tid = oic::Thread::getCurrentId();
	
		if(contexts.find(tid) == contexts.end())
			return *(contexts[tid] = new GLContext{});

		return *contexts[tid];
	}

}