#include "utils/thread.hpp"
#include "graphics/command/command_list.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"
#include "graphics/memory/depth_texture.hpp"
#include "graphics/gl_graphics.hpp"
#include "graphics/gl_context.hpp"
#include "graphics/memory/gl_framebuffer.hpp"
#include "graphics/memory/gl_texture_object.hpp"
#include "graphics/memory/swapchain.hpp"
#include "graphics/memory/upload_buffer.hpp"
#include "graphics/shader/descriptors.hpp"
#include "system/system.hpp"

namespace ignis {

	void Graphics::wait() {

		if (!isThreadEnabled())
			return;

		//Wait for all pending commands and then signal upload buffers to free that memory

		GLContext &ctx = data->getContext();

		if (ctx.fences.size()) {

			List<UploadBuffer*> uploads;
			uploads.reserve(16);

			for (auto &gobj : graphicsObjects)
				if (gobj.first.type == GPUObjectType::UPLOAD_BUFFER)
					uploads.push_back((UploadBuffer*)gobj.second);

			for (auto &fence : ctx.fences) {

				glClientWaitSync(fence.second.sync, GL_SYNC_FLUSH_COMMANDS_BIT, u64_MAX);

				fence.second.call();

				glDeleteSync(fence.second.sync);

				for (auto *res : fence.second.objects)
					res->loseRef();

				for (auto *upl : uploads)
					upl->end(fence.first);

			}

			ctx.fences.clear();
		}


	}

	Graphics::~Graphics() {
		wait();
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

	bool Graphics::supportsFormat(GPUFormat) const {
		//TODO: Query support
		return true;
	}

	GraphicsApi Graphics::getCurrentApi() const {
		return GraphicsApi::OPENGL;
	}

	List<GPUObject*> Graphics::executeInternal(const List<CommandList*> &commands, bool isIndepedentExecution) {


		oicAssert("Graphics::execute can't be ran on a suspended graphics thread", enabledThreads[oic::Thread::getCurrentId()].enabled);

		++data->executionId;

		//Updates VAOs and FBOs that have been added/released
		data->updateContext(*this);

		List<GPUObject*> resources;

		for (CommandList *cl : commands)
			cl->execute(resources);

		//Make sure that all immediate handles are converted to frame independent

		if (isIndepedentExecution) {
			data->storeContext(resources);
			return {};
		}

		return resources;
	}

	void Graphics::presentToCpuInternal(
		const List<CommandList*> &commands,
		TextureObject *target,
		UploadBuffer *result,
		PresentToCpuCallback callback,
		void *callbackInstance,
		Vec3u16 size, Vec3u16 offset,
		u8 mip,
		u16 layer,
		bool isStencil
	) {

		//Validate arguments

		const auto &info = target->getInfo();

		oicAssert("Mip out of bounds", mip < info.mips);
		oicAssert("Mip out of bounds", layer < info.layers);
		oicAssert("Offset out of bounds", ((offset.cast<Vec3u32>() + size.cast<Vec3u32>()) < info.mipSizes[mip].cast<Vec3u32>()).all());

		if (!size.all())
			size = info.mipSizes[mip] - offset;

		GLenum format{}, type{};
		usz stride{};

		if(auto *depth = dynamic_cast<DepthTexture*>(target)) {

			if (isStencil) {

				if (!FormatHelper::hasStencil(depth->getFormat()))
					oic::System::log()->fatal("Depth texture missing stencil, couldn't present to cpu");

				format = GL_STENCIL_INDEX;
				type = GL_UNSIGNED_BYTE;
				stride = 1;
			}
			else {

				switch (depth->getFormat()) {

					case DepthFormat::D16:		stride = 2; type = GL_UNSIGNED_SHORT;	break;

					case DepthFormat::D32:
					case DepthFormat::D24_S8:
					case DepthFormat::D24:		stride = 4; type = GL_UNSIGNED_INT;		break;

					case DepthFormat::D32F_S8:
					case DepthFormat::D32F:		stride = 4; type = GL_FLOAT;			break;

					default:
						oic::System::log()->fatal("Invalid depth format");
				}

				format = GL_DEPTH_COMPONENT;
			}
		}

		else if(isStencil)
			oic::System::log()->fatal("Stencil requested for cpu present, but color image doesn't have one");

		else {

			switch (FormatHelper::getChannelCount(info.format)) {
			case 1:	format = GL_RED;		break;
			case 2:	format = GL_RG;			break;
			case 3:	format = GL_RGB;		break;
			case 4:	format = GL_RGBA;		break;
			}

			switch (FormatHelper::getStrideBytes(info.format)) {
			case 1:	type = GL_BYTE;		break;
			case 2:	type = GL_SHORT;	break;
			case 4:	type = GL_INT;		break;
			}

			if (FormatHelper::isFloatingPoint(info.format))
				type = GL_FLOAT;

			else if (!FormatHelper::isSigned(info.format))
				type |= 1;

			stride = FormatHelper::getSizeBytes(info.format);
		}

		//Execute and allocate memory for the frame
		
		List<GPUObject*> objects = executeInternal(commands, false);

		Pair<u64, u64> allocation = result->allocate(data->executionId, nullptr, target->size(mip, isStencil), 1);

		oicAssert("Out of memory exception", allocation.second != u64_MAX);

		//Ensure our resources are counted

		auto it0 = std::find(objects.begin(), objects.end(), target);

		if (it0 == objects.end()) objects.push_back(target);

		auto it1 = std::find(objects.begin(), objects.end(), result);

		if (it1 == objects.end()) objects.push_back(result);

		//Copy to GPU buffer

		//TODO: Optimize bind call if possible

		auto buf = result->getInfo().buffers.find(allocation.first);

		oicAssert("UploadBuffer somehow disappeared", buf != result->getInfo().buffers.end());

		glBindBuffer(GL_PIXEL_PACK_BUFFER, buf->second->getExtendedData()->handle);

		if (!size.all())
			size = info.mipSizes[mip] - offset;

		if (info.textureType == TextureType::TEXTURE_1D_ARRAY)
			offset.y = layer;
		else
			offset.z = std::max(layer, offset.z);

		usz textureSize = size.prod<usz>() * stride;

		glGetTextureSubImage(
			target->getData()->handle,
			mip,
			offset.x, offset.y, offset.z,
			size.x, size.y, size.z,
			format,
			type,
			GLsizei(textureSize),
			(void*) allocation.second
		);

		//Finish

		data->storeContext(
			objects, 
			callbackInstance, callback, 
			target, result, allocation,
			offset,
			size,
			layer,
			mip,
			isStencil
		);
	}

	void Graphics::execute(const List<CommandList*> &commands) {

		//TODO: lock mutex (also in present and presentToCpu)

		executeInternal(commands, true);

		//TODO: unlock mutex
	}

	//Present framebuffer to swapchain

	void Graphics::present(
		Framebuffer *intermediate, Swapchain *swapchain,
		const List<CommandList*> &commands
	) {

		if (!swapchain)
			oic::System::log()->fatal("Couldn't present; invalid intermediate or swapchain");

		if (!intermediate)
			oic::System::log()->warn("Presenting without an intermediate is valid but won't provide any results to the swapchain");

		if (intermediate && intermediate->getInfo().size != swapchain->getInfo().size)
			oic::System::log()->fatal("Couldn't present; swapchain and intermediate aren't same size");

		//Execute

		GLContext &ctx = data->getContext();

		List<GPUObject*> objects = executeInternal(commands, false);

		//Ensure our swapchain and intermediate don't suddenly disappear

		auto it0 = std::find(objects.begin(), objects.end(), intermediate);

		if (it0 == objects.end()) objects.push_back(intermediate);

		auto it1 = std::find(objects.begin(), objects.end(), swapchain);

		if (it1 == objects.end()) objects.push_back(swapchain);

		//Copy intermediate to backbuffer

		ctx.boundApi.framebuffer = nullptr;
		glxBeginRenderPass(data->getContext(), {}, 0);

		if (intermediate) {

			Vec2u16 size = intermediate->getInfo().size;

			oicAssert("Framebuffer should have 1 render texture to copy", intermediate->size());
			oicAssert("Framebuffer should have a proper resolution before blit", size.all());

			//Bind backbuffer

			//TODO: Fix this! it should use a shader!

			if (ctx.enableScissor) {
				glDisable(GL_SCISSOR_TEST);
				ctx.enableScissor = false;
			}

			glxSetViewport(ctx, swapchain->getInfo().size.cast<Vec2u32>(), {});

			ctx.boundObjects[GL_READ_FRAMEBUFFER] = intermediate->getId();
			ctx.boundObjects[GL_DRAW_FRAMEBUFFER] = {};

			glBlitNamedFramebuffer(
				intermediate->getData()->handle,
				0,
				0, 0, size.x, size.y,
				0, size.y, size.x, 0,
				GL_COLOR_BUFFER_BIT, GL_LINEAR
			);
		}

		swapchain->present();
		++ctx.frameId;

		//Insert fence and store data

		data->storeContext(objects);

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

		//Execute

		GLContext &ctx = data->getContext();

		List<GPUObject*> objects = executeInternal(commands, false);

		//Ensure our swapchain and intermediate don't suddenly disappear

		auto it0 = std::find(objects.begin(), objects.end(), intermediate);

		if (it0 == objects.end()) objects.push_back(intermediate);

		auto it1 = std::find(objects.begin(), objects.end(), swapchain);

		if (it1 == objects.end()) objects.push_back(swapchain);

		//Copy intermediate to backbuffer

		ctx.boundApi.framebuffer = {};
		glxBeginRenderPass(data->getContext(), {}, 0);

		if (intermediate) {

			if (ctx.enableScissor) {
				glDisable(GL_SCISSOR_TEST);
				ctx.enableScissor = false;
			}

			glxSetViewport(ctx, swapchain->getInfo().size.cast<Vec2u32>(), {});

			ctx.boundObjects[GL_READ_FRAMEBUFFER] = intermediate->getId();
			ctx.boundObjects[GL_DRAW_FRAMEBUFFER] = {};

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

		//Place fence

		data->storeContext(objects);
	}

	void Graphics::Data::updateContext(Graphics &g) {

		GLContext &ctx = getContext();
		ctx.executionId = g.getData()->executionId;

		//Update status of previous fences

		if (ctx.fences.size()) {

			List<UploadBuffer*> uploads;
			uploads.reserve(16);

			for (auto &gobj : g)
				if (gobj.first.type == GPUObjectType::UPLOAD_BUFFER)
					uploads.push_back((UploadBuffer*)gobj.second);

			List<u64> syncs;
			syncs.reserve(ctx.fences.size());

			for (auto &fence : ctx.fences) {

				GLenum type = glClientWaitSync(fence.second.sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

				if (type == GL_TIMEOUT_EXPIRED || type == GL_WAIT_FAILED)
					continue;

				fence.second.call();

				glDeleteSync(fence.second.sync);
				syncs.push_back(fence.first);

				for (auto *res : fence.second.objects)
					res->loseRef();

				for (auto *upl : uploads)
					upl->end(fence.first);

			}

			for (auto &sync : syncs)
				ctx.fences.erase(sync);
		}

		//Acquire resources from id (since they might be destroyed)

		auto *fb = ctx.framebufferId.get<Framebuffer>();
		auto *pb = ctx.primitiveBufferId.get<PrimitiveBuffer>();
		auto *p = ctx.pipelineId.get<Pipeline>();

		ctx.bound.descriptors.clear();
		ctx.boundApi.descriptors.clear();

		for (auto &elem : ctx.descriptorsIds) {
			ctx.bound.descriptors.push_back(elem.get<Descriptors>());
			ctx.boundApi.descriptors.push_back(elem.get<Descriptors>());
		}

		ctx.bound.pipeline = ctx.boundApi.pipeline = p;
		ctx.bound.framebuffer = ctx.boundApi.framebuffer = fb;
		ctx.bound.primitiveBuffer = ctx.boundApi.primitiveBuffer = pb;

		//Unbind if the resource stopped existing

		for(auto &bound : ctx.boundObjects)
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

	void Graphics::Data::storeContext(
		const List<GPUObject*> &resources, 
		void *callbackObjectPtr, 
		void (*callbackPtr)(void*, UploadBuffer*, const Pair<u64, u64>&, TextureObject*, const Vec3u16&, const Vec3u16&, u16, u8, bool),
		TextureObject *gpuOutput,
		UploadBuffer *cpuOutput,
		const Pair<u64, u64> &allocation,
		Vec3u16 offset,
		Vec3u16 size,
		u16 layer,
		u8 mip,
		bool isStencil
	) {

		GLContext &ctx = getContext();

		//Store frame independent ids

		ctx.framebufferId = getGPUObjectId(ctx.boundApi.framebuffer);
		ctx.primitiveBufferId = getGPUObjectId(ctx.boundApi.primitiveBuffer);
		ctx.pipelineId = getGPUObjectId(ctx.boundApi.pipeline);

		ctx.descriptorsIds.clear();

		for(auto &elem : ctx.boundApi.descriptors)
			ctx.descriptorsIds.push_back(getGPUObjectId(elem));

		//Increment refcounters, to ensure they don't disappear during execution

		for (auto *res : resources)
			res->addRef();

		//Queue fence

		ctx.fences[ctx.executionId] = { 
			glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0), 
			resources,
			callbackObjectPtr,
			callbackPtr,
			gpuOutput,
			cpuOutput,
			allocation,
			offset,
			size,
			layer,
			mip,
			isStencil
		};

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