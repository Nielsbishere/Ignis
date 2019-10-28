#pragma once
#include "../graphics_object.hpp"
#include "graphics/memory/buffer_layout.hpp"
#include "graphics/shader/pipeline_layout.hpp"

namespace ignis {

	enum class TopologyMode : u8;
	enum class FillMode : u8;
	enum class WindMode : u8;
	enum class CullMode : u8;
	enum class ShaderStage : u8;

	class Pipeline : public GraphicsObject {

	public:

		enum class Flag : u8 {
			OPTIMIZE = 1 << 0
		};

		struct Rasterizer {

			FillMode fill;
			CullMode cull;
			WindMode winding;

			Rasterizer(FillMode fill = FillMode::FILL, CullMode cull = CullMode::BACK, WindMode wind = WindMode::CCW): 
				fill(fill), cull(cull), winding(wind) {}

		};

		struct MSAA {

			u32 samples;

			MSAA(u32 samples = {}) : samples(samples) {}
		};

		struct Info {

			List<BufferAttributes> attributeLayout;
			List<HashMap<ShaderStage, Buffer>> passes;

			PipelineLayout pipelineLayout;

			Flag flag;
			TopologyMode topology;
			Rasterizer rasterizer;
			MSAA msaa;

			Info(
				Flag f, 
				const List<BufferAttributes> &attributeLayout, 
				const HashMap<ShaderStage, Buffer> &passes,
				const PipelineLayout &pipelineLayout,
				MSAA msaa = {},
				TopologyMode topology = TopologyMode::TRIANGLE_LIST, 
				Rasterizer rasterizer = {}
			) : 
				flag(f), attributeLayout(attributeLayout), topology(topology),
				passes{ passes }, pipelineLayout(pipelineLayout),
				rasterizer(rasterizer), msaa(msaa) { }

			Info(
				Flag f,
				const List<BufferAttributes> &attributeLayout,
				const List<HashMap<ShaderStage, Buffer>> &passes,
				const PipelineLayout &pipelineLayout,
				MSAA msaa = {},
				TopologyMode topology = TopologyMode::TRIANGLE_LIST,
				Rasterizer rasterizer = {}
			) : 
				flag(f), attributeLayout(attributeLayout), topology(topology), 
				passes(passes),  pipelineLayout(pipelineLayout), 
				rasterizer(rasterizer), msaa(msaa) { }

		};

		apimpl struct Data;

		apimpl Pipeline(Graphics &g, const String &name, const Info &info);
		apimpl ~Pipeline();

		const Info &getInfo() const { return info; }
		Data *getData() { return data; }

	private:

		Info info;
		Data *data{};
	};

}