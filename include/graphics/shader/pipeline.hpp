#pragma once
#include "graphics/graphics_object.hpp"
#include "graphics/memory/buffer_layout.hpp"
#include "graphics/shader/pipeline_layout.hpp"
#include "types/vec.hpp"

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

		struct BlendState {

			Vec4f32 blendFactor;

			enum class LogicOp : u8 {

				CLEAR, AND, AND_REV, COPY,
				AND_INV, NO_OP, XOR, OR,
				NOR, EQUIV, INV, OR_REV,
				COPY_INV, OR_INV, NAND, SET

			} logicOp;

			enum class WriteMask : u8 {

				R = 1, B = 2, G = 4, A = 8,

				NONE = 0x0,
				ALL = 0xF

			} writeMask;

			enum class BlendOp : u8 {

				ADD, SUBTRACT,
				REV_SUBTRACT,
				MIN, MAX

			} blendOp, alphaBlendOp;

			enum class Blend : u8 {

				ZERO, ONE, SRC, SRC_REV,
				DST, DST_REV, SRC_ALPHA, SRC_ALPHA_REV,
				DST_ALPHA, DST_ALPHA_REV, FACTOR, FACTOR_REV,
				FACTOR_ALPHA, FACTOR_ALPHA_REV, SRC_ALPHA_SAT, SRC1,
				SRC1_REV, SRC1_ALPHA, SRC1_ALPHA_REV

			} srcBlend, dstBlend, alphaSrcBlend, alphaDstBlend;

			bool blendEnable;

			BlendState(
				bool blendEnable = false,
				BlendOp blendOp = BlendOp::ADD,
				Blend srcBlend = Blend::ZERO,
				Blend dstBlend = Blend::ONE,
				BlendOp alphaBlendOp = BlendOp::ADD,
				Blend alphaSrcBlend = Blend::ZERO,
				Blend alphaDstBlend = Blend::ONE,
				WriteMask writeMask = WriteMask::ALL,
				LogicOp logicOp = LogicOp::NO_OP,
				const Vec4f32 &blendFactor = {}
			) :
				blendFactor(blendFactor), logicOp(logicOp), writeMask(writeMask), blendOp(blendOp), alphaBlendOp(alphaBlendOp),
				srcBlend(srcBlend), dstBlend(dstBlend), alphaSrcBlend(alphaSrcBlend), alphaDstBlend(alphaDstBlend),
				blendEnable(blendEnable) {}

			static BlendState alphaBlend(
				WriteMask mask = WriteMask::ALL,
				LogicOp logicOp = LogicOp::NO_OP,
				const Vec4f32 &blendFactor = {}
			) {
				return BlendState(
					true,
					BlendOp::ADD, Blend::ONE, Blend::SRC_ALPHA_REV,
					BlendOp::ADD, Blend::ONE, Blend::SRC_ALPHA_REV,
					mask, logicOp, blendFactor
				);
			}

			inline bool logOpEnable() const { return logicOp != LogicOp::NO_OP; }

		};

		struct MSAA {

			u32 samples;			//How many samples are taken for this pipeline
			f32 minSampleShading;	//How to resolve textures with a MSAA texture (0 = off, closer to one is smoother)

			MSAA(u32 samples = {}, f32 minSampleShading = {}) : samples(samples), minSampleShading(minSampleShading) {}
		};

		struct Info {

			List<HashMap<ShaderStage, Buffer>> passes;

			PipelineLayout pipelineLayout;

			Flag flag;

			//Graphics attributes

			List<BufferAttributes> attributeLayout{};

			TopologyMode topology{};
			Rasterizer rasterizer{};
			BlendState blendState{};
			MSAA msaa{};

			//Compute attributes

			Vec3u32 groupSize{};

			//Graphics

			Info(
				Flag f, 
				const List<BufferAttributes> &attributeLayout, 
				const HashMap<ShaderStage, Buffer> &passes,
				const PipelineLayout &pipelineLayout,
				MSAA msaa = {},
				Rasterizer rasterizer = {},
				BlendState blendState = {},
				TopologyMode topology = TopologyMode::TRIANGLE_LIST
			) : 
				passes{ passes }, pipelineLayout(pipelineLayout), flag(f),
				attributeLayout(attributeLayout), topology(topology),
				rasterizer(rasterizer), blendState(blendState), msaa(msaa) { }

			Info(
				Flag f,
				const List<BufferAttributes> &attributeLayout,
				const List<HashMap<ShaderStage, Buffer>> &passes,
				const PipelineLayout &pipelineLayout,
				MSAA msaa = {},
				Rasterizer rasterizer = {},
				BlendState blendState = {},
				TopologyMode topology = TopologyMode::TRIANGLE_LIST
			) : 
				passes(passes), pipelineLayout(pipelineLayout), flag(f),
				attributeLayout(attributeLayout), topology(topology), 
				rasterizer(rasterizer), blendState(blendState), msaa(msaa) { }

			//Compute

			Info(
				Flag f, 
				const Buffer &computeShader,
				const PipelineLayout &pipelineLayout,
				const Vec3u32 &groupSize
			) : 
				passes{ { { ShaderStage::COMPUTE, computeShader } } }, 
				pipelineLayout(pipelineLayout), flag(f),
				groupSize(groupSize) { }

			Info(
				Flag f,
				const List<Buffer> &computeShaders,
				const PipelineLayout &pipelineLayout,
				const Vec3u32 &groupSize
			) : 
				passes(computeShaders.size()), pipelineLayout(pipelineLayout),
				flag(f), groupSize(groupSize) {
			
				size_t i{};

				for (auto &buf : computeShaders)
					passes[i++] = { { ShaderStage::COMPUTE, buf } };
			}

		};

		apimpl struct Data;

		apimpl Pipeline(Graphics &g, const String &name, const Info &info);

		const Info &getInfo() const { return info; }
		Data *getData() { return data; }

		inline bool isCompute() const { return info.groupSize[0]; }
		inline bool isGraphics() const { return info.attributeLayout.size(); }

	private:

		apimpl ~Pipeline();

		Info info;
		Data *data{};
	};

}