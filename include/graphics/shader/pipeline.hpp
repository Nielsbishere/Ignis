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

			List<HashMap<ShaderStage, Buffer>> passes;

			PipelineLayout pipelineLayout;

			Flag flag;

			//Graphics attributes

			List<BufferAttributes> attributeLayout{};

			TopologyMode topology{};
			Rasterizer rasterizer{};
			MSAA msaa{};

			//Compute attributes

			Vec3u groupSize{};

			//Graphics

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

			//Compute

			Info(
				Flag f, 
				const Buffer &computeShader,
				const PipelineLayout &pipelineLayout,
				Vec3u groupSize
			) : 
				flag(f), passes { { { ShaderStage::COMPUTE, computeShader } } },
				pipelineLayout(pipelineLayout), groupSize(groupSize) { }

			Info(
				Flag f,
				const List<Buffer> &computeShaders,
				const PipelineLayout &pipelineLayout,
				Vec3u groupSize
			) : 
				flag(f), passes(computeShaders.size()),
				pipelineLayout(pipelineLayout), groupSize(groupSize) {
			
				size_t i{};

				for (auto &buf : computeShaders)
					passes[i++] = { { ShaderStage::COMPUTE, buf } };
			}

		};

		apimpl struct Data;

		apimpl Pipeline(Graphics &g, const String &name, const Info &info);
		apimpl ~Pipeline();

		const Info &getInfo() const { return info; }
		Data *getData() { return data; }

		inline bool isCompute() const { return info.groupSize[0]; }
		inline bool isGraphics() const { return info.attributeLayout.size(); }

	private:

		Info info;
		Data *data{};
	};

}