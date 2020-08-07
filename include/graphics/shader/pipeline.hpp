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
		u8 pad{};

		Rasterizer(CullMode cull = CullMode::BACK, FillMode fill = FillMode::FILL, WindMode wind = WindMode::CCW): 
			fill(fill), cull(cull), winding(wind) {}

		inline bool operator==(const Rasterizer &other) const {
			return *(const u32*)&fill == *(const u32*)&other.fill;
		}
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

		//Stride of stencil is 4x1, so one stencil fits in an u32
		//front and back fit into u64
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
		u8 pad{};

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

		inline bool operator==(const DepthStencil &other) const {
			return 
				*(const u64*)&front == *(const u64*)&other.front &&
				*(const u64*)&stencilMask == *(const u64*)&other.stencilMask;
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

		inline bool operator==(const BlendState &other) const {
			return 
				blendFactor == other.blendFactor &&
				*(const u64*)logicOp == *(const u64*)&other.logicOp &&
				blendEnable == other.blendEnable;
		}

	};

	struct MSAA {

		u32 samples;			//How many samples are taken for this pipeline
		f32 minSampleShading;	//How to resolve textures with a MSAA texture (0 = off, closer to one is smoother)

		MSAA(u32 samples = 1, f32 minSampleShading = {}) : samples(samples), minSampleShading(minSampleShading) {}

		inline bool operator==(const MSAA &other) const {
			return *(const u64*)&samples == *(const u64*)&other.samples;
		}
	};

	class Pipeline : public GPUObject {

	public:

		enum class Flag : u8 {

			NONE						= 0,
			IS_PARENT					= 1 << 0,		//Allows the pipeline to be used as a base (to reduce compilation time)
			DISABLE_OPTIMIZATION		= 1 << 1,

			RT_LIBRARY					= 1 << 2,		//Specifies a library that can be combined to form one raytracing pipeline
			RT_DISABLE_TRIANGLES		= 1 << 3,		//Skip triangle checking in raytracing (for if procedural geometry is used)
			RT_DISABLE_PROCEDURAL		= 1 << 4,		//Skip AABB checking (for if triangles are used)
			RT_PLACEHOLDER_ANYHIT		= 1 << 5,		//Always use an anyhit shader even if one is not provided
			RT_PLACEHOLDER_CLOSESTHIT	= 1 << 6,		//Always use a closesthit shader even if one is not provided
			RT_PLACEHOLDER_MISS			= 1 << 7,		//Always use a miss shader even if one is not provided

			RT_FLAGS					= 0xFC
		};

		struct Info {

			HashMap<String, Buffer> binaries;
			HashMap<ShaderStage, Pair<String, String>> stages;

			const PipelineLayout *pipelineLayout;

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

			//Parent pipeline (must have IS_PARENT flag set)
			Pipeline *parent{};

			//Graphics

			Info(
				Flag f, 
				const List<BufferAttributes> &attributeLayout, 
				const HashMap<String, Buffer> &binaries,
				const HashMap<ShaderStage, Pair<String, String>> &stages,
				const PipelineLayout *pipelineLayout,
				MSAA msaa = {},
				DepthStencil depthStencil = {},
				Rasterizer rasterizer = {},
				BlendState blendState = {},
				TopologyMode topology = TopologyMode::TRIANGLE_LIST
			) : 
				stages(stages), binaries(binaries), pipelineLayout(pipelineLayout), flag(f),
				attributeLayout(attributeLayout), topology(topology), depthStencil(depthStencil),
				rasterizer(rasterizer), blendState(blendState), msaa(msaa)
			{

				for (auto &stage : stages)

					if (stage.first > ShaderStage::FRAGMENT)
						oic::System::log()->fatal("Invalid graphics shaders (only accepting vert, geom, tesc, tese, frag");

					else if(binaries.find(stage.second.first) == binaries.end() && binaries.size())
						oic::System::log()->fatal("Stage pointed to an invalid binary");
			}

			Info(
				Flag f, 
				const List<BufferAttributes> &attributeLayout, 
				const String &binaryLocation,
				const Buffer &binary,
				const HashMap<ShaderStage, String> &cstages,
				const PipelineLayout *pipelineLayout,
				MSAA msaa = {},
				DepthStencil depthStencil = {},
				Rasterizer rasterizer = {},
				BlendState blendState = {},
				TopologyMode topology = TopologyMode::TRIANGLE_LIST
			) : 
				binaries{ { binaryLocation, binary } }, pipelineLayout(pipelineLayout), flag(f),
				attributeLayout(attributeLayout), topology(topology), depthStencil(depthStencil),
				rasterizer(rasterizer), blendState(blendState), msaa(msaa)
			{ 
				for (auto &stage : cstages) {

					if (stage.first > ShaderStage::FRAGMENT)
						oic::System::log()->fatal("Invalid graphics shaders (only accepting vert, geom, tesc, tese, frag");

					stages[stage.first] = { binaryLocation, stage.second };
				}
			}

			//Compute

			Info(
				Flag f, 
				const String &binaryLocation,
				const Buffer &computeShader,
				const PipelineLayout *pipelineLayout,
				const Vec3u32 &groupSize,
				const String &entryPoint = "main"
			) : 
				stages{{ ShaderStage::COMPUTE, { binaryLocation, entryPoint } }}, 
				binaries { { binaryLocation, computeShader } },
				pipelineLayout(pipelineLayout), flag(f),
				groupSize(groupSize) { }

			//Helpers

			inline bool hasStage(ShaderStage stage) const {
				return stages.find(stage) != stages.end();
			}

			inline bool isCompute() const {
				return stages.size() == 1 && hasStage(ShaderStage::COMPUTE);
			}

			inline bool isRaytracing() const {
				return stages.size() >= 1 && u8(stages.begin()->first) & u8(ShaderStage::PROPERTY_IS_RAYTRACING);
			}

			inline bool isGraphics() const {
				return !isCompute() && !isRaytracing();
			}

			inline bool operator==(const Info &other) const {

				if (isCompute() && !other.isCompute())
					return false;

				if (isGraphics() && !other.isGraphics())
					return false;

				if (isRaytracing() && !other.isRaytracing())
					return false;

				if(
					flag != other.flag || parent != other.parent || 
					pipelineLayout != other.pipelineLayout || stages != other.stages ||
					binaries.size() != other.binaries.size()
				)
					return false;

				//Cheap compare of binary (not just all bytes)
				for (auto &bin : binaries) {

					auto it = other.binaries.find(bin.first);

					if (it == other.binaries.end())
						return false;

					if (bin.second.size() != it->second.size())
						return false;
				}

				if(!isGraphics())
					return groupSize == other.groupSize;

				return 
					attributeLayout == other.attributeLayout &&
					topology == other.topology && depthStencil == other.depthStencil &&
					rasterizer == other.rasterizer && blendState == other.blendState &&
					msaa == other.msaa;
			}

		};

		apimpl struct Data;

		apimpl Pipeline(Graphics &g, const String &name, const Info &info);

		inline const Info &getInfo() const { return info; }
		inline Data *getData() { return data; }

		inline bool isCompute() const { return info.isCompute(); }
		inline bool isRaytracing() const { return info.isRaytracing(); }
		inline bool isGraphics() const { return info.isGraphics(); }

	private:

		apimpl ~Pipeline();

		Info info;
		Data *data{};
	};

	enumFlagOverloads(Pipeline::Flag);

	using PipelineRef = GraphicsObjectRef<Pipeline>;
}