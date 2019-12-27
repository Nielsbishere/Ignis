#include "utils/thread.hpp"
#include "graphics/command/command_list.hpp"
#include "graphics/command/command_ops.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"
#include "graphics/gl_graphics.hpp"
#include "graphics/gl_context.hpp"
#include "graphics/surface/gl_framebuffer.hpp"
#include "graphics/surface/swapchain.hpp"
#include "graphics/shader/descriptors.hpp"
#include "system/system.hpp"
#include "utils/hash.hpp"
#include "../res/shaders/blit_image.frag.hpp"
#include "../res/shaders/blit_image.vert.hpp"

namespace ignis {

	Graphics::~Graphics() { 

		data->blitBuffer->loseRef();
		data->blitPipeline->loseRef();

		data->blitBuffer = nullptr;
		data->blitPipeline = nullptr;

		release();
		destroy(data);
	}

	Graphics::Graphics() { 

		data = new Graphics::Data(); 
		init();

		data->blitBuffer = new GPUBuffer(
			*this, NAME("Blit uniform buffer"),
			GPUBuffer::Info(sizeof(BlitImageIntoFramebuffer), GPUBufferType::UNIFORM, GPUMemoryUsage::CPU_WRITE)
		);

		Buffer
			vertexShader = { blitImageVert, blitImageVert + sizeof(blitImageVert) - 1 },
			fragmentShader = { blitImageFrag, blitImageFrag + sizeof(blitImageFrag) - 1 };

		data->blitPipeline = new Pipeline(
			*this, NAME("Blit pipeline"),
			Pipeline::Info(

				Pipeline::Flag::OPTIMIZE,
				List<BufferAttributes>(),	// No primitive buffer needed

				{
					{ ShaderStage::VERTEX, vertexShader },
					{ ShaderStage::FRAGMENT, fragmentShader }
				},

				PipelineLayout()			//We manage pipeline layout ourselves, so no definition
			)
		);

	}

	GraphicsApi Graphics::getCurrentApi() const {
		return GraphicsApi::OPENGL;
	}

	CommandAvailability Graphics::getCommandAvailability(CommandOp op) {
		
		if (op >> CMD_PROPERTY_TECHNIQUE_SHIFT)
			return CommandAvailability::UNSUPPORTED;

		return CommandAvailability::SUPPORTED;
	}

	void Graphics::execute(const List<CommandList*> &commands) {

		//Updates VAOs and FBOs that have been added/released
		data->updateContext();

		for (CommandList *cl : commands)
			cl->execute();
	}

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

			Vec2u32 size = intermediate->getInfo().size;
			auto rt = intermediate->getData()->renderTextures;

			oicAssert("Framebuffer should have 1 render texture to copy", rt.size() != 0);

			//Bind backbuffer

			glxBeginRenderPass(ctx, 0);
			ctx.currentFramebuffer = nullptr;

			glxSetViewportAndScissor(ctx, swapchain->getInfo().size, {});

			//Bind blit pipeline

			glxBindPipeline(ctx, data->blitPipeline);

			//Flip framebuffer and resolve MSAA

			auto *blitBuffer = data->blitBuffer;

			BlitImageIntoFramebuffer udata = { Vec4f32(0, 0, 0, 1), Vec4f32(f32(size.x), f32(size.y), 1, -1), intermediate->getInfo().samples };

			std::memcpy(blitBuffer->getBuffer(), &udata, sizeof(udata));
			blitBuffer->flush(0, sizeof(udata));

			auto &boundBuf = ctx.boundByBase[GL_UNIFORM_BUFFER];	//uniform buffer 0 is our data

			if (boundBuf.handle != blitBuffer->getData()->handle) {
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, blitBuffer->getData()->handle);
				boundBuf = { blitBuffer->getData()->handle, 0, 0 };
			}

			//Bind resolve texture

			auto &boundTex = ctx.boundByBase[GL_TEXTURE];	//texture 0 is our image

			if (boundTex.handle != rt[0]) {
				glBindTextureUnit(0, rt[0]);
				boundTex.handle = rt[0];
			}

			//Dispatch triangle data baked into shader

			glDrawArrays(GL_TRIANGLES, 0, 6);

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