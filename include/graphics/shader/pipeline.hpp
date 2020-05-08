#pragma once
#include "graphics/graphics.hpp"
#include "graphics/memory/buffer_layout.hpp"
#include "graphics/shader/pipeline_layout.hpp"
#include "types/vec.hpp"

namespace ignis {

	enum class TopologyMode : u8;
	enum class FillMode : u8;
	enum class WindMode : u8;
	enum class CullMode : u8;
	enum class ShaderStage : u8;

	struct Rasterizer {

		FillMode fill;
		CullMode cull;
		WindMode winding;

		Rasterizer(CullMode cull = CullMode::BACK, FillMode fill = FillMode::FILL, WindMode wind = WindMode::CCW): 
			fill(fill), cull(cull), winding(wind) {}

	};

	enum class CompareOp : u8 {
		NV, LE, EQ, LEQ, GR, NEQ, GEQ, AL
	};
	
	enum class StencilOp : u8 {
		KEEP, ZERO, REPL,
		INC_CLAMP, DEC_CLAMP,
		INV, INC_WRAP, DEC_WRAP
	};

	struct DepthStencil {

		struct Stencil {

			StencilOp fail, pass, depthFail;
			CompareOp compare;

			constexpr Stencil(
				StencilOp fail = StencilOp::KEEP,
				StencilOp pass = StencilOp::KEEP,
				StencilOp depthFail = StencilOp::KEEP,
				CompareOp compare = CompareOp::AL
			) :
				fail(fail), pass(pass), depthFail(depthFail), compare(compare) {}

		} front{}, back{};

		u8 stencilMask = 0xFF, stencilWriteMask = 0xFF, stencilReference = 0x00;

		CompareOp depthCompare = CompareOp::GR;
		bool enableDepthRead{}, enableDepthWrite{}, enableStencilTest{};

		//Constructors

		static constexpr DepthStencil depth(
			CompareOp depthCompare = CompareOp::GR,
			bool depthWrite = true,
			bool depthRead = true
		) {
			depthCompare = depthWrite && !depthRead ? CompareOp::NV : depthCompare;
			depthRead = (depthWrite && !depthRead) || depthRead;

			return {
				{}, {},
				0xFF, 0xFF, 0x00,
				depthCompare, depthRead,
				depthWrite, false
			};
		}

		static constexpr DepthStencil depthStencil(
			CompareOp depthCompare = CompareOp::GR,
			bool depthWrite = true, bool depthRead = true,
			Stencil frontAndBack = {},
			u8 stencilMask = 0xFF, u8 stencilWriteMask = 0xFF,
			u8 stencilReference = 0x00
		) {
			depthCompare = depthWrite && !depthRead ? CompareOp::NV : depthCompare;
			depthRead = (depthWrite && !depthRead) || depthRead;

			return {
				frontAndBack, frontAndBack,
				stencilMask, stencilWriteMask, stencilReference,
				depthCompare, depthRead,
				depthWrite, true
			};
		}

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

		//Helpers

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
			LogicOp logicOp = LogicOp::NO_OP
		) {
			return BlendState(
				true,
				BlendOp::ADD, Blend::ONE, Blend::SRC_ALPHA_REV,
				BlendOp::ADD, Blend::ONE, Blend::SRC_ALPHA_REV,
				mask, logicOp, {}
			);
		}

		static BlendState subpixelAlphaBlend(
			WriteMask mask = WriteMask::ALL,
			LogicOp logicOp = LogicOp::NO_OP
		) {
			return BlendState(
				true,
				BlendOp::ADD, Blend::SRC1, Blend::SRC1_REV,
				BlendOp::ADD, Blend::SRC1_ALPHA, Blend::SRC1_ALPHA_REV,
				mask, logicOp, {}
			);
		}

		inline bool logOpEnable() const { return logicOp != LogicOp::NO_OP; }

	};

	struct MSAA {

		u32 samples;			//How many samples are taken for this pipeline
		f32 minSampleShading;	//How to resolve textures with a MSAA texture (0 = off, closer to one is smoother)

		MSAA(u32 samples = {}, f32 minSampleShading = {}) : samples(samples), minSampleShading(minSampleShading) {}
	};

	class Pipeline : public GPUObject {

	public:

		enum class Flag : u8 {
			OPTIMIZE = 1 << 0
		};

		struct Info {

			HashMap<ShaderStage, Buffer> stages;

			PipelineLayout pipelineLayout;

			Flag flag;

			//Graphics attributes

			List<BufferAttributes> attributeLayout{};

			TopologyMode topology{};
			DepthStencil depthStencil{};
			Rasterizer rasterizer{};
			BlendState blendState{};
			MSAA msaa{};

			//Compute attributes

			Vec3u32 groupSize{};

			//Graphics

			Info(
				Flag f, 
				const List<BufferAttributes> &attributeLayout, 
				const HashMap<ShaderStage, Buffer> &stages,
				const PipelineLayout &pipelineLayout,
				MSAA msaa = {},
				DepthStencil depthStencil = {},
				Rasterizer rasterizer = {},
				BlendState blendState = {},
				TopologyMode topology = TopologyMode::TRIANGLE_LIST
			) : 
				stages(stages), pipelineLayout(pipelineLayout), flag(f),
				attributeLayout(attributeLayout), topology(topology), depthStencil(depthStencil),
				rasterizer(rasterizer), blendState(blendState), msaa(msaa) { }

			//Compute

			Info(
				Flag f, 
				const Buffer &computeShader,
				const PipelineLayout &pipelineLayout,
				const Vec3u32 &groupSize
			) : 
				stages{{ ShaderStage::COMPUTE, computeShader }}, 
				pipelineLayout(pipelineLayout), flag(f),
				groupSize(groupSize) { }

		};

		apimpl struct Data;

		apimpl Pipeline(Graphics &g, const String &name, const Info &info);

		const Info &getInfo() const { return info; }
		Data *getData() { return data; }

		inline bool isCompute() const { return info.groupSize[0]; }
		inline bool isGraphics() const { return !isCompute(); }

	private:

		apimpl ~Pipeline();

		Info info;
		Data *data{};
	};

}