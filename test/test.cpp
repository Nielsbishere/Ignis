#include "graphics/command/commands.hpp"
#include "graphics/surface/swapchain.hpp"
#include "graphics/surface/framebuffer.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/shader_buffer.hpp"
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
	//Texture *tex2D{};

	//Cleanup resources

	~TestViewportInterface() {
		destroy(/*tex2D, */uniforms, pipeline, descriptors, mesh, cl, intermediate, s);
	}

	//Create resources

	void init(ViewportInfo *vp) final override {

		//Create MSAA render target and window swapchain

		s = new Swapchain(g, NAME("Swapchain"), Swapchain::Info{ vp, false, DepthFormat::NONE });

		intermediate =  new Framebuffer(
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

		const Vec2f mask = { 1, 1 };

		uniforms = new ShaderBuffer(
			g, NAME("Test pipeline uniform buffer"),
			ShaderBuffer::Info(
				HashMap<String, ShaderBuffer::Layout>{ { NAME("mask"), { 0, mask } } },
				GPUBufferType::UNIFORM, GPUMemoryUsage::LOCAL
			)
		);

		//Create texture

		const List<List<u32>> rgba = {
			{ 0xFF00FFFF, 0x00FFFFFF },
			{ 0xFFFF00FF, 0xF00FFFFF }
		};

		/*tex2D = new Texture(
			g, NAME("Test texture"),
			Texture::Info(
				Vec2u(2, 2), GPUFormat::RGBA8, TextureMip::AUTO, 
				GPUMemoryUsage::LOCAL, rgba
			)
		);*/

		//Load shader code

		Buffer vert, frag;
		bool success =	oic::System::files()->read("./shaders/test.vert.spv", vert);
		success		&=	oic::System::files()->read("./shaders/test.frag.spv", frag);

		if (!success)
			oic::System::log()->fatal("Couldn't find shaders");

		//Create descriptors that should be bound

		PipelineLayout pipelineLayout(
			RegisterLayout(NAME("Test"), 0, GPUBufferType::UNIFORM, 0, ACCESS_VERTEX, uniforms->size())
		);

		descriptors = new Descriptors(
			g, NAME("Test descriptors"), 
			Descriptors::Info(pipelineLayout, { { 0, uniforms } })
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
				Pipeline::MSAA(intermediate->getInfo().samples)	//TODO: Do validation with RT

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