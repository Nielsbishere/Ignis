#include "graphics/command/commands.hpp"
#include "graphics/surface/swapchain.hpp"
#include "graphics/surface/framebuffer.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/shader_buffer.hpp"
#include "graphics/memory/texture.hpp"
#include "graphics/shader/sampler.hpp"
#include "graphics/shader/pipeline.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/enums.hpp"
#include "graphics/graphics.hpp"
#include "system/viewport_manager.hpp"
#include "system/viewport_interface.hpp"
#include "system/local_file_system.hpp"
#include "utils/hash.hpp"

using namespace ignis;
using namespace oic;
using namespace cmd;

struct TestViewportInterface : public ViewportInterface {

	//Construct graphics instance

	Graphics g;

	//Resources

	Surface *s{}, *intermediate{};
	CommandList *cl{};
	PrimitiveBuffer *mesh{};
	Descriptors *descriptors{};
	Pipeline *pipeline{};
	GPUBuffer *uniforms{};
	Texture *tex2D{};
	Sampler *samp{};

	//Cleanup resources

	~TestViewportInterface() {
		destroy(samp, tex2D, uniforms, pipeline, descriptors, mesh, cl, intermediate, s);
	}

	//Create resources

	void init(ViewportInfo *vp) final override {

		//Create MSAA render target and window swapchain

		s = new Swapchain(g, NAME("Swapchain"), Swapchain::Info{ vp, false, DepthFormat::NONE });

		intermediate = new Framebuffer(
			g, NAME("Framebuffer"),
			Surface::Info(
				{ GPUFormat::RGBA16f }, DepthFormat::NONE, false, 8
			)
		);

		//Create primitive buffer

		List<BufferAttributes> attrib{ { GPUFormat::RG32f } };

		const List<Vec2f> vboBuffer{
			{ 0.5, -0.5 }, { -1, -1 },
			{ -1, 1 }, { 0.5, 0.5 }
		};

		const List<u8> iboBuffer{
			0,3,2, 2,1,0
		};

		mesh = new PrimitiveBuffer(g, NAME("Test mesh"),
			PrimitiveBuffer::Info(
				BufferLayout(vboBuffer, attrib[0]), 
				BufferLayout(iboBuffer, GPUFormat::R8u)
			)
		);

		//Create uniform buffer

		const Vec3f mask = { 1, 1, 1 };

		uniforms = new ShaderBuffer(
			g, NAME("Test pipeline uniform buffer"),
			ShaderBuffer::Info(
				HashMap<String, ShaderBuffer::Layout>{ { NAME("mask"), { 0, mask } } },
				GPUBufferType::UNIFORM, GPUMemoryUsage::LOCAL
			)
		);

		//Create texture

		const u32 rgba0[2][2] = {
			{ 0xFFFF00FF, 0xFF00FFFF },	//1,0,1, 0,1,1
			{ 0xFFFFFF00, 0xFFFFFFFF }	//1,1,0, 1,1,1
		};

		const u32 rgba1[1][1] = {
			{ 0xFFBFBFBF }				//0.75,0.75,0.75
		};

		tex2D = new Texture(
			g, NAME("Test texture"),
			Texture::Info(
				List<Grid2D<u32>>{
					rgba0, rgba1
				}, 
				GPUFormat::RGBA8, GPUMemoryUsage::LOCAL
			)
		);

		//Create sampler

		samp = new Sampler(
			g, NAME("Test sampler"), Sampler::Info()
		);

		//Load shader code

		Buffer vert, frag;
		bool success =	oic::System::files()->read("./shaders/test.vert.spv", vert);
		success		&=	oic::System::files()->read("./shaders/test.frag.spv", frag);

		if (!success)
			oic::System::log()->fatal("Couldn't find shaders");

		//Create descriptors that should be bound

		PipelineLayout pipelineLayout(

			RegisterLayout(
				NAME("Test"), 0, GPUBufferType::UNIFORM, 0,
				ACCESS_FRAGMENT, uniforms->size()
			),

			RegisterLayout(
				NAME("test"), 1, SamplerType::SAMPLER_2D, 0,
				ACCESS_FRAGMENT
			)
		);

		auto descriptorsInfo = Descriptors::Info(pipelineLayout, {});
		descriptorsInfo.resources[0] = uniforms;
		descriptorsInfo.resources[1] = GPUSubresource(samp, tex2D);

		descriptors = new Descriptors(
			g, NAME("Test descriptors"), 
			descriptorsInfo
		);

		//Create pipeline (shader and render states)

		pipeline = new Pipeline(
			g, NAME("Test pipeline"),

			Pipeline::Info(

				Pipeline::Flag::OPTIMIZE,

				attrib,

				{ 
					{ ShaderStage::VERTEX, vert }, 
					{ ShaderStage::FRAGMENT, frag } 
				},

				pipelineLayout,
				Pipeline::MSAA(intermediate->getInfo().samples)

				//TODO:
				//DepthStencil()
				//BlendState()
				//RenderPass
				//Parent pipeline / allow parenting (optional)
			)
		);

		//Create command list and store our commands

		cl = new CommandList(g, NAME("Command list"), CommandList::Info(1_KiB));

		cl->add(

			//Clear and bind MSAA

			SetClearColor(Vec4f { 0.586f, 0.129f, 0.949f, 1.0f }),
			BeginSurface(intermediate),

			//TODO: BeginRenderPass instead of BeginSurface

			//Draw primitive

			BindDescriptors(descriptors),
			BindPipeline(pipeline),
			BindPrimitiveBuffer(mesh),
			DrawInstanced(mesh->elements()),

			//Present to surface

			EndSurface(),
			BlitSurface(intermediate, s, Vec4u(), Vec4u(), BlitSurface::COLOR, BlitSurface::LINEAR),
			Present()
		);
	}

	//Update size of surfaces

	void resize(const Vec2u &size) final override {
		intermediate->onResize(size);
		s->onResize(size);
	}

	//Execute commandList

	void render() final override {
		g.execute(cl);
	}

};

//Create window and wait for exit

int main() {

	System::viewportManager()->create(
		ViewportInfo("Test window", {}, {}, 0, new TestViewportInterface())
	);

	//TODO: Better way of stalling than this; like interrupt
	while (System::viewportManager()->size())
		System::wait(250_ms);

	return 0;
}