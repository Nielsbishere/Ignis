#include "graphics/command/commands.hpp"
#include "graphics/surface/swapchain.hpp"
#include "graphics/surface/framebuffer.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/shader/pipeline.hpp"
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

	Graphics g;
	CommandList *cl{};
	Surface *s{}, *intermediate{};
	PrimitiveBuffer *mesh{};
	Pipeline *pipeline{};

	~TestViewportInterface() {
		destroy(mesh);
		destroy(cl);
		destroy(intermediate);
		destroy(s);
	}

	void init(ViewportInfo *vp) final override {

		s = new Swapchain(g, NAME("Swapchain"), Swapchain::Info{ vp, false, DepthFormat::NONE });

		intermediate = 
			new Framebuffer(
				g, NAME("Framebuffer"),
				Surface::Info(
					{ GPUFormat::RGBA16f }, DepthFormat::NONE, false, 1, 8
				)
			);

		const List<Vec2f> vboBuffer{
			{ 0.5, -0.5 }, { -1, -1 },
			{ -1, 1 }, { 0.5, 0.5 }
		};

		const List<u8> iboBuffer{
			0,3,2, 2,1,0
		};

		List<BufferAttributes> attrib{ { GPUFormat::RG32f } };

		Buffer vert, frag;
		bool success =	oic::System::files()->read("./shaders/test.vert.spv", vert);
		success		&=	oic::System::files()->read("./shaders/test.frag.spv", frag);

		if (!success)
			oic::System::log()->fatal("Couldn't find shaders");

		pipeline = new Pipeline(
			g, NAME("Test pipeline"),

			Pipeline::Info(

				Pipeline::Flag::OPTIMIZE,

				attrib,

				{ 
					{ ShaderStage::VERTEX, vert }, 
					{ ShaderStage::FRAGMENT, frag } 
				}

				//DepthStencil()
				//BlendState()
				//MSAA
				//RenderPass
				//Shaders
				//Parent pipeline / allow parenting (optional)
				//Viewports/scissor (optional)
			)
		);

		mesh = new PrimitiveBuffer(g, NAME("Test mesh"),
			PrimitiveBuffer::Info(
				BufferLayout(vboBuffer, attrib[0]), 
				BufferLayout(iboBuffer, GPUFormat::R8u)
			)
		);

		cl = new CommandList(g, NAME("Command list"), CommandList::Info(1_KiB));

		cl->add(

			//Clear and bind MSAA

			SetClearColor(Vec4f { 0.586f, 0.129f, 0.949f, 1.0f }),
			BeginSurface(intermediate),

			//Draw primitive
			//TODO: Bind render pass
			BindPipeline(pipeline),
			BindPrimitiveBuffer(mesh),
			DrawInstanced(mesh->elements()),

			//Present to surface

			EndSurface(),
			BlitSurface(intermediate, s, Vec4u(), Vec4u(), BlitSurface::COLOR, BlitSurface::LINEAR),
			Present()
		);
	}

	void resize(const Vec2u &size) final override {
		intermediate->onResize(size);
		s->onResize(size);
	}

	void render() final override {
		g.execute(cl);
	}

};

int main() {

	System::viewportManager()->create(
		ViewportInfo("Test window", {}, {}, 0, new TestViewportInterface())
	);

	//TODO: Better way of stalling than this; like interrupt
	while (System::viewportManager()->size())
		System::wait(1_s);

	return 0;
}