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

	Graphics::Graphics() {
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

	void Graphics::Data::removeTexture(GLuint tex) {

		//TODO: Remove buffer

		for (auto &ctx : contexts) {

			for (auto &bound : ctx.second.bound)
				if (bound.second == tex)
					bound.second = 0;

			for (auto &bound : ctx.second.boundByBase)
				if (bound.second.handle == tex)
					bound.second.handle = 0;
		}

	}

	void Graphics::execute(const List<CommandList*> &commands) {

		//Updates VAOs and FBOs that have been added/released
		data->updateContext();

		for (CommandList *cl : commands)
			cl->execute();
	}

	//Present framebuffer to swapchain

	void Graphics::present(
		Framebuffer *intermediate, Swapchain *swapchain,
		const List<CommandList*> &commands
	) {

		if (!swapchain)
			oic::System::log()->fatal("Couldn't present; invalid intermediate or swapchain");

		if(!intermediate)
			oic::System::log()->warn("Presenting without an intermediate is valid but won't provide any results to the swapchain");

		if(intermediate && intermediate->getInfo().size != swapchain->getInfo().size)
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
			glBlitNamedFramebuffer(
				ctx.bound[GL_READ_FRAMEBUFFER] = intermediate->getData()->index, 
				ctx.bound[GL_DRAW_FRAMEBUFFER] = 0,
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
		Texture *intermediate, Swapchain *swapchain,
		const List<CommandList*> &commands
	) {

		if (!swapchain)
			oic::System::log()->fatal("Couldn't present; invalid intermediate or swapchain");

		if(!intermediate)
			oic::System::log()->warn("Presenting without an intermediate is valid but won't provide any results to the swapchain");

		if(intermediate->getInfo().textureType != TextureType::TEXTURE_2D)
			oic::System::log()->fatal("Couldn't present; intermediate texture has to be 2D");

		auto size = intermediate->getInfo().dimensions.cast<Vec2u16>();

		if(intermediate && size != swapchain->getInfo().size)
			oic::System::log()->fatal("Couldn't present; swapchain and intermediate aren't same size");

		GLContext &ctx = data->getContext();

		swapchain->bind();

		execute(commands);

		//Copy intermediate to backbuffer
		if (intermediate) {
			glxSetViewportAndScissor(ctx, swapchain->getInfo().size.cast<Vec2u32>(), {});
			glBlitNamedFramebuffer(
				ctx.bound[GL_READ_FRAMEBUFFER] = intermediate->getData()->framebuffer,
				ctx.bound[GL_DRAW_FRAMEBUFFER] = 0,
				0, 0, size.x, size.y,
				0, 0, size.x, size.y,
				GL_COLOR_BUFFER_BIT, GL_NEAREST
			);
		}

		swapchain->present();
		++ctx.frameId;
	}

	//Keep track of objects for updating gl contexts
	//It will delete VAOs and unbind bound objects

	void Graphics::onAddOrErase(GraphicsObject *go, bool isDeleted) {

		//TODO: This is not called on construction because the type isn't fully constructed

		if (!isDeleted) {

			if (go->canCast<PrimitiveBuffer>())
				data->primitiveBuffers[(PrimitiveBuffer*)go];

			return;
		}


		for (auto &context : data->contexts) {
			if (go->canCast<Pipeline>()) {

				if (context.second.pipeline == (Pipeline*)go)
					context.second.pipeline = nullptr;

			} else if (go->canCast<Descriptors>()) {

				if (context.second.descriptors == (Descriptors*)go)
					context.second.descriptors = nullptr;

			} else if (go->canCast<Framebuffer>()) {

				if (context.second.currentFramebuffer == (Framebuffer*)go)
					context.second.currentFramebuffer = nullptr;

			}
		} 
		
		
		if (go->canCast<PrimitiveBuffer>()) {

			auto *pb = (PrimitiveBuffer*)go;

			data->primitiveBuffers.erase(pb);

			//Remove all referenced VAOs in contexts next time they update

			for (auto &context : data->contexts) {

				if (context.second.primitiveBuffer == pb)
					context.second.primitiveBuffer = nullptr;

				if (context.second.vaos.find(pb) != context.second.vaos.end())
					context.second.deletedVaos.push_back(pb);
			}
		}

	}

	void Graphics::Data::updateContext() {

		GLContext &context = getContext();

		//Clean up left over VAOs

		for (auto &del : context.deletedVaos) {
			glDeleteVertexArrays(1, &context.vaos[del]);
			context.vaos.erase(del);
		}

		context.deletedVaos.clear();

	}

	void Graphics::Data::destroyContext() {

		GLContext &context = getContext();
		
		for(auto &vao : context.vaos)
			glDeleteVertexArrays(1, &vao.second);

		contexts.erase(oic::Thread::getCurrentId());
	}

	GLContext &Graphics::Data::getContext() {
		return contexts[oic::Thread::getCurrentId()];
	}

}