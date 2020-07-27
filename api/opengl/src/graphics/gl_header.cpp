#include "graphics/enums.hpp"
#include "system/log.hpp"
#include "system/system.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/framebuffer.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"
#include "graphics/memory/gl_texture_object.hpp"
#include "graphics/shader/gl_sampler.hpp"
#include "graphics/gl_graphics.hpp"
#include "graphics/gl_context.hpp"

HashMap<String, void**> ignis::glFunctionNames = HashMap<String, void**>();

template<typename T>
static T appendGlFunc(const String &s, T &t) {
	ignis::glFunctionNames[s] = (void**)&t;
	return nullptr;
}

#define GL_FUNC(x, y) PFN##y##PROC x = appendGlFunc(#x, x)

#include "graphics/gl_functions.hpp"
#include "graphics/shader/gl_pipeline.hpp"

using namespace ignis;

//Enums

GLenum glxDepthFormat(DepthFormat format) {

	switch (format) {

		case DepthFormat::D16:		return GL_DEPTH_COMPONENT16;
		case DepthFormat::D32:		return GL_DEPTH_COMPONENT32;

		case DepthFormat::D24_S8:	return GL_DEPTH24_STENCIL8;
		case DepthFormat::D24:		return GL_DEPTH_COMPONENT24;

		case DepthFormat::D32F:		return GL_DEPTH_COMPONENT32F;
		case DepthFormat::D32F_S8:	return GL_DEPTH32F_STENCIL8;

		default:
			oic::System::log()->fatal("Invalid depth format");
			return GL_DEPTH_COMPONENT32F;
	}

}

GLenum glxColorFormat(GPUFormat format){

	switch (format.value) {

		case GPUFormat::R8:		return GL_R8;
		case GPUFormat::RG8:	return GL_RG8;
		case GPUFormat::BGR8:
		case GPUFormat::RGB8:	return GL_RGB8;
		case GPUFormat::BGRA8:
		case GPUFormat::RGBA8:	return GL_RGBA8;

		case GPUFormat::R16:	return GL_R16;
		case GPUFormat::RG16:	return GL_RG16;
		case GPUFormat::RGB16:	return GL_RGB16;
		case GPUFormat::RGBA16:	return GL_RGBA16;

		case GPUFormat::R8s:	return GL_R8_SNORM;
		case GPUFormat::RG8s:	return GL_RG8_SNORM;
		case GPUFormat::BGR8s:
		case GPUFormat::RGB8s:	return GL_RGB8_SNORM;
		case GPUFormat::BGRA8s:
		case GPUFormat::RGBA8s:	return GL_RGBA8_SNORM;

		case GPUFormat::R16s:	return GL_R16_SNORM;
		case GPUFormat::RG16s:	return GL_RG16_SNORM;
		case GPUFormat::RGB16s:	return GL_RGB16_SNORM;
		case GPUFormat::RGBA16s:return GL_RGBA16_SNORM;

		case GPUFormat::R8u:	return GL_R8UI;
		case GPUFormat::RG8u:	return GL_RG8UI;
		case GPUFormat::BGR8u:
		case GPUFormat::RGB8u:	return GL_RGB8UI;
		case GPUFormat::BGRA8u:
		case GPUFormat::RGBA8u:	return GL_RGBA8UI;

		case GPUFormat::R16u:	return GL_R16UI;
		case GPUFormat::RG16u:	return GL_RG16UI;
		case GPUFormat::RGB16u:	return GL_RGB16UI;
		case GPUFormat::RGBA16u:return GL_RGBA16UI;

		case GPUFormat::R32u:	return GL_R32UI;
		case GPUFormat::RG32u:	return GL_RG32UI;
		case GPUFormat::RGB32u:	return GL_RGB32UI;
		case GPUFormat::RGBA32u:return GL_RGBA32UI;

		case GPUFormat::R8i:	return GL_R8I;
		case GPUFormat::RG8i:	return GL_RG8I;
		case GPUFormat::BGR8i:
		case GPUFormat::RGB8i:	return GL_RGB8I;
		case GPUFormat::BGRA8i:
		case GPUFormat::RGBA8i:	return GL_RGBA8I;

		case GPUFormat::R16i:	return GL_R16I;
		case GPUFormat::RG16i:	return GL_RG16I;
		case GPUFormat::RGB16i:	return GL_RGB16I;
		case GPUFormat::RGBA16i:return GL_RGBA16I;

		case GPUFormat::R32i:	return GL_R32I;
		case GPUFormat::RG32i:	return GL_RG32I;
		case GPUFormat::RGB32i:	return GL_RGB32I;
		case GPUFormat::RGBA32i:return GL_RGBA32I;

		case GPUFormat::R16f:	return GL_R16F;
		case GPUFormat::RG16f:	return GL_RG16F;
		case GPUFormat::RGB16f:	return GL_RGB16F;
		case GPUFormat::RGBA16f:return GL_RGBA16F;

		case GPUFormat::R32f:	return GL_R32F;
		case GPUFormat::RG32f:	return GL_RG32F;
		case GPUFormat::RGB32f:	return GL_RGB32F;
		case GPUFormat::RGBA32f:return GL_RGBA32F;

		case GPUFormat::sBGR8:
		case GPUFormat::sRGB8:	return GL_SRGB8;

		case GPUFormat::sBGRA8:
		case GPUFormat::sRGBA8:	return GL_SRGB8_ALPHA8;

		case GPUFormat::R64f: case GPUFormat::R64u: case GPUFormat::R64i:
		case GPUFormat::RG64f: case GPUFormat::RG64u: case GPUFormat::RG64i:
		case GPUFormat::RGB64f: case GPUFormat::RGB64u: case GPUFormat::RGB64i:
		case GPUFormat::RGBA64f: case GPUFormat::RGBA64u: case GPUFormat::RGBA64i:
			oic::System::log()->fatal("OpenGL doesn't support 64-bit buffers");
			return GL_RGBA8;

		default:
			oic::System::log()->fatal("Invalid depth format");
			return GL_RGBA8;

	}

}

GLenum glxBufferType(GPUBufferType format) {

	switch (format) {

		case GPUBufferType::UNIFORM:				return GL_UNIFORM_BUFFER;
		case GPUBufferType::VERTEX:					return GL_ARRAY_BUFFER;
		case GPUBufferType::INDEX:					return GL_ELEMENT_ARRAY_BUFFER;

		case GPUBufferType::STRUCTURED:
		case GPUBufferType::STORAGE:				return GL_SHADER_STORAGE_BUFFER;

		case GPUBufferType::INDIRECT_DRAW:			return GL_DRAW_INDIRECT_BUFFER;
		case GPUBufferType::INDIRECT_DISPATCH:		return GL_DISPATCH_INDIRECT_BUFFER;

		default:
			oic::System::log()->fatal("Invalid buffer type");
			return GL_UNIFORM_BUFFER;
	}
}

GLenum glxGpuFormatType(GPUFormat type) {

	const GPUFormatType t = FormatHelper::getType(type);
	const usz stride = FormatHelper::getStrideBits(type);

	const bool isSigned = FormatHelper::isSigned(type);
	
	if (t == GPUFormatType::FLOAT) {

		if (stride == 16)
			return GL_HALF_FLOAT;

		else if (stride == 32)
			return GL_FLOAT;

		else goto error;
	}

	switch (stride) {

		case 8:
			return isSigned ? GL_BYTE : GL_UNSIGNED_BYTE;

		case 16:
			return isSigned ? GL_SHORT : GL_UNSIGNED_SHORT;

		case 32:
			return isSigned ? GL_INT : GL_UNSIGNED_INT;
	}

error:

	oic::System::log()->fatal("Unsupported GPU format");
	return {};
}

GLenum glxGpuDataFormat(GPUFormat format) {
	
	switch (FormatHelper::getChannelCount(format)) {

	case 1:
		return GL_RED;

	case 2:
		return GL_RG;

	case 3:
		return FormatHelper::flipRGB(format) ? GL_BGR : GL_RGB;

	case 4:
		return FormatHelper::flipRGB(format) ? GL_BGRA : GL_RGBA;

	}

	oic::System::log()->fatal("Unsupported GPU format");
	return {};
}

GLenum glxTopologyMode(TopologyMode topo) {

	switch (topo) {

		case TopologyMode::POINT_LIST:			return GL_POINTS;
		case TopologyMode::LINE_LIST:			return GL_LINES;
		case TopologyMode::LINE_STRIP:			return GL_LINE_STRIP;
		case TopologyMode::TRIANGLE_LIST:		return GL_TRIANGLES;
		case TopologyMode::TRIANGLE_STRIP:		return GL_TRIANGLE_STRIP;
		case TopologyMode::LINE_LIST_ADJ:		return GL_LINES_ADJACENCY;
		case TopologyMode::LINE_STRIP_ADJ:		return GL_LINE_STRIP_ADJACENCY;
		case TopologyMode::TRIANGLE_LIST_ADJ:	return GL_TRIANGLES_ADJACENCY;
		case TopologyMode::TRIANGLE_STRIP_ADJ:	return GL_TRIANGLE_STRIP_ADJACENCY;

		default:
			oic::System::log()->fatal("Unsupported topology mode");
			return {};
	}
}

GLenum glxShaderStage(ShaderStage stage) {

	if (u8(stage) & 0x40)
		oic::System::log()->fatal("OpenGL doesn't natively support raytracing");

	switch (stage) {

		case ShaderStage::VERTEX:			return GL_VERTEX_SHADER;
		case ShaderStage::GEOMETRY:			return GL_GEOMETRY_SHADER;
		case ShaderStage::TESS_CTRL:		return GL_TESS_CONTROL_SHADER;
		case ShaderStage::TESS_EVAL:		return GL_TESS_EVALUATION_SHADER;
		case ShaderStage::FRAGMENT:			return GL_FRAGMENT_SHADER;
		case ShaderStage::COMPUTE:			return GL_COMPUTE_SHADER;
		case ShaderStage::TASK_EXT:			return GL_TASK_SHADER_NV;
		case ShaderStage::MESH_EXT:			return GL_MESH_SHADER_NV;

		default:
			oic::System::log()->fatal("Invalid shader stage");
			return {};
	}
}

GLenum glxTextureType(TextureType type) {

	switch (type) {

		case TextureType::TEXTURE_CUBE:			return GL_TEXTURE_CUBE_MAP;
		case TextureType::TEXTURE_1D:			return GL_TEXTURE_1D;
		case TextureType::TEXTURE_2D:			return GL_TEXTURE_2D;
		case TextureType::TEXTURE_3D:			return GL_TEXTURE_3D;
		case TextureType::TEXTURE_MS:			return GL_TEXTURE_2D_MULTISAMPLE;

		case TextureType::TEXTURE_CUBE_ARRAY:	return GL_TEXTURE_CUBE_MAP_ARRAY;
		case TextureType::TEXTURE_1D_ARRAY:		return GL_TEXTURE_1D_ARRAY;
		case TextureType::TEXTURE_2D_ARRAY:		return GL_TEXTURE_2D_ARRAY;
		case TextureType::TEXTURE_MS_ARRAY:		return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;

		default:
			oic::System::log()->fatal("Invalid texture type");
			return {};
	}
}

GLenum glxSamplerMode(SamplerMode mode) {

	switch (mode) {
		case SamplerMode::CLAMP_EDGE:			return GL_CLAMP_TO_EDGE;
		case SamplerMode::MIRROR_CLAMP_EDGE:	return GL_MIRROR_CLAMP_TO_EDGE;
		case SamplerMode::CLAMP_BORDER:			return GL_CLAMP_TO_BORDER;
		case SamplerMode::REPEAT:				return GL_REPEAT;
		case SamplerMode::MIRROR_REPEAT:		return GL_MIRRORED_REPEAT;
	}

	oic::System::log()->fatal("Invalid sampler mode");
	return {};
}

GLenum glxSamplerMag(SamplerMag mag) {

	if (mag == SamplerMag::LINEAR)				return GL_LINEAR;
	else if (mag == SamplerMag::NEAREST)		return GL_NEAREST;

	oic::System::log()->fatal("Invalid sampler mag");
	return {};
}

GLenum glxSamplerMin(SamplerMin min) {

	switch (min) {
		case SamplerMin::LINEAR_MIPS:			return GL_LINEAR_MIPMAP_LINEAR;
		case SamplerMin::LINEAR_MIPS_NEAREST:	return GL_LINEAR_MIPMAP_NEAREST;
		case SamplerMin::LINEAR:				return GL_LINEAR;
		case SamplerMin::NEAREST:				return GL_NEAREST;
		case SamplerMin::NEAREST_MIPS_LINEAR:	return GL_NEAREST_MIPMAP_LINEAR;
		case SamplerMin::NEAREST_MIPS:			return GL_NEAREST_MIPMAP_NEAREST;
	}

	oic::System::log()->fatal("Invalid sampler mag");
	return {};
}

using LogicOp = BlendState::LogicOp;
using BlendOp = BlendState::BlendOp;
using Blend = BlendState::Blend;

GLenum glxLogicOp(LogicOp logicOp) {

	switch (logicOp) {
		case LogicOp::CLEAR:					return GL_CLEAR;
		case LogicOp::AND:						return GL_AND;
		case LogicOp::AND_REV:					return GL_AND_REVERSE;
		case LogicOp::COPY:						return GL_COPY;
		case LogicOp::AND_INV:					return GL_AND_INVERTED;
		case LogicOp::NO_OP:					return GL_NOOP;
		case LogicOp::XOR:						return GL_XOR;
		case LogicOp::OR:						return GL_OR;
		case LogicOp::NOR:						return GL_NOR;
		case LogicOp::EQUIV:					return GL_EQUIV;
		case LogicOp::INV:						return GL_INVERT;
		case LogicOp::OR_REV:					return GL_OR_REVERSE;
		case LogicOp::COPY_INV:					return GL_COPY_INVERTED;
		case LogicOp::OR_INV:					return GL_OR_INVERTED;
		case LogicOp::NAND:						return GL_NAND;
		case LogicOp::SET:						return GL_SET;
	}

	oic::System::log()->fatal("Invalid logic op");
	return {};
}

GLenum glxBlendOp(BlendOp blendOp) {

	switch (blendOp) {
		case BlendOp::ADD:						return GL_FUNC_ADD;
		case BlendOp::SUBTRACT:					return GL_FUNC_SUBTRACT;
		case BlendOp::REV_SUBTRACT:				return GL_FUNC_REVERSE_SUBTRACT;
		case BlendOp::MIN:						return GL_MIN;
		case BlendOp::MAX:						return GL_MAX;
	}

	oic::System::log()->fatal("Invalid logic op");
	return {};
}

GLenum glxBlend(Blend blend) {

	switch (blend) {
		case Blend::ZERO:						return GL_ZERO;
		case Blend::ONE:						return GL_ONE;
		case Blend::SRC:						return GL_SRC_COLOR;
		case Blend::SRC_REV:					return GL_ONE_MINUS_SRC_COLOR;
		case Blend::DST:						return GL_DST_COLOR;
		case Blend::DST_REV:					return GL_ONE_MINUS_DST_COLOR;
		case Blend::SRC_ALPHA:					return GL_SRC_ALPHA;
		case Blend::SRC_ALPHA_REV:				return GL_ONE_MINUS_SRC_ALPHA;
		case Blend::DST_ALPHA:					return GL_DST_ALPHA;
		case Blend::DST_ALPHA_REV:				return GL_ONE_MINUS_DST_ALPHA;
		case Blend::FACTOR:						return GL_CONSTANT_COLOR;
		case Blend::FACTOR_REV:					return GL_ONE_MINUS_CONSTANT_COLOR;
		case Blend::FACTOR_ALPHA:				return GL_CONSTANT_ALPHA;
		case Blend::FACTOR_ALPHA_REV:			return GL_ONE_MINUS_CONSTANT_ALPHA;
		case Blend::SRC_ALPHA_SAT:				return GL_SRC_ALPHA_SATURATE;
		case Blend::SRC1:						return GL_SRC1_COLOR;
		case Blend::SRC1_REV:					return GL_ONE_MINUS_SRC1_COLOR;
		case Blend::SRC1_ALPHA:					return GL_SRC1_ALPHA;
		case Blend::SRC1_ALPHA_REV:				return GL_ONE_MINUS_SRC1_ALPHA;
	}

	oic::System::log()->fatal("Invalid logic op");
	return {};
}

GLenum glxCompareFunc(CompareOp compareOp) {

	switch (compareOp) {
		case CompareOp::NV:						return GL_NEVER;
		case CompareOp::LE:						return GL_LESS;
		case CompareOp::EQ:						return GL_EQUAL;
		case CompareOp::LEQ:					return GL_LEQUAL;
		case CompareOp::GR:						return GL_GREATER;
		case CompareOp::NEQ:					return GL_NOTEQUAL;
		case CompareOp::GEQ:					return GL_GEQUAL;
		case CompareOp::AL:						return GL_ALWAYS;
	}

	oic::System::log()->fatal("Invalid compare op");
	return {};
}

GLenum glxStencilOp(StencilOp stencilOp) {

	switch (stencilOp) {
		case StencilOp::KEEP:					return GL_KEEP;
		case StencilOp::ZERO:					return GL_ZERO;
		case StencilOp::REPL:					return GL_REPLACE;
		case StencilOp::INC_CLAMP:				return GL_INCR;
		case StencilOp::DEC_CLAMP:				return GL_DECR;
		case StencilOp::INV:					return GL_INVERT;
		case StencilOp::INC_WRAP:				return GL_INCR_WRAP;
		case StencilOp::DEC_WRAP:				return GL_DECR_WRAP;
	}

	oic::System::log()->fatal("Invalid stencil op");
	return {};
}

//Functionality

void glxBeginRenderPass(
	GLContext &ctx, const GPUObjectId &framebuffer, GLuint i
) {
	if (ctx.boundObjects[GL_DRAW_FRAMEBUFFER] != framebuffer) {
		ctx.boundObjects[GL_DRAW_FRAMEBUFFER] = framebuffer;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, i);
	}
}

void glxClearFramebuffer(GLContext &ctx, GLuint fbo, GLuint index, const cmd::SetClearColor &clearColor) {

	using namespace cmd;

	if (ctx.clearColor.type == SetClearColor::Type::FLOAT)
		glClearNamedFramebufferfv(fbo, GL_COLOR, index, clearColor.rgbaf.arr);

	else if (ctx.clearColor.type == SetClearColor::Type::UNSIGNED_INT)
		glClearNamedFramebufferuiv(fbo, GL_COLOR, index, clearColor.rgbau.arr);

	else
		glClearNamedFramebufferiv(fbo, GL_COLOR, index, clearColor.rgbai.arr);

}

void glxDebugMessage(
	GLenum source, GLenum type, GLuint, GLenum severity,
	GLsizei, const GLchar *message, const void *
) {

	oic::LogLevel logLevel = oic::LogLevel::DEBUG;

	switch (severity) {

	case GL_DEBUG_SEVERITY_HIGH:
		logLevel = oic::LogLevel::FATAL;
		break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		logLevel = oic::LogLevel::ERROR;
		break;

	case GL_DEBUG_SEVERITY_LOW:
		logLevel = oic::LogLevel::WARN;
		break;
	}

	if (type == GL_DEBUG_TYPE_PERFORMANCE)
		logLevel = oic::LogLevel::PERFORMANCE;

	static const HashMap<GLenum, String> sources = {
		{ GL_DEBUG_SOURCE_API, "API" },
		{ GL_DEBUG_SOURCE_WINDOW_SYSTEM, "Windows system" },
		{ GL_DEBUG_SOURCE_SHADER_COMPILER, "Shader compiler" },
		{ GL_DEBUG_SOURCE_THIRD_PARTY, "Third party" },
		{ GL_DEBUG_SOURCE_APPLICATION, "App" },
		{ GL_DEBUG_SOURCE_OTHER, "Other" }
	};

	static const HashMap<GLenum, const c8 *> types = {
		{ GL_DEBUG_TYPE_ERROR, "Error" },
		{ GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "Deprecated behavior" },
		{ GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "Undefined behavior" },
		{ GL_DEBUG_TYPE_PORTABILITY, "Portability" },
		{ GL_DEBUG_TYPE_PERFORMANCE, "Performance" },
		{ GL_DEBUG_TYPE_OTHER, "Other" }
	};

	auto it = types.find(type);
	auto itt = sources.find(source);

	if (it == types.end() || itt == sources.end())
		return;

	oic::System::log()->println(logLevel, "OpenGL (", itt->second, ") ", it->second, ": ", message);
}

template<GLenum type, typename GlGetIv, typename GlGetInfoLog>
bool glCheckLog(GlGetIv glGetIv, GlGetInfoLog glGetInfoLog, GLuint handle, String &str) {

	GLint success{};

	glGetIv(handle, type, &success);

	if (!success) {

		GLint logLength{};	//Including null terminator
		glGetIv(handle, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength) {
			str = String(usz(logLength) - 1, ' ');
			glGetInfoLog(handle, logLength, &logLength, str.data());
		}

		return true;
	}

	return false;
}

bool glxCheckShaderLog(GLuint shader, String &str) {
	return glCheckLog<GL_COMPILE_STATUS>(glGetShaderiv, glGetShaderInfoLog, shader, str);
}

bool glxCheckProgramLog(GLuint program, String &str) {
	return glCheckLog<GL_LINK_STATUS>(glGetProgramiv, glGetProgramInfoLog, program, str);
}

void glxBindPipeline(GLContext &ctx, Pipeline *pipeline) {

	glUseProgram(pipeline->getData()->handle);

	if (!pipeline->isGraphics())
		return;

	auto &r = pipeline->getInfo().rasterizer;
	auto &b = pipeline->getInfo().blendState;
	auto &m = pipeline->getInfo().msaa;
	auto &d = pipeline->getInfo().depthStencil;

	auto &r0 = ctx.currRaster;
	auto &b0 = ctx.currBlend;
	auto &d0 = ctx.currDepth;

	//Rasterizer

	if (r0.cull != r.cull) {

		if (u8(r0.cull) && !u8(r.cull))
			glDisable(GL_CULL_FACE);

		if (!u8(r0.cull) && u8(r.cull))
			glEnable(GL_CULL_FACE);

		if (u8(r.cull))
			glCullFace(r.cull == CullMode::BACK ? GL_BACK : GL_FRONT);

		r0.cull = r.cull;
	}

	if (r0.winding != r.winding && u8(r.cull)) {
		glFrontFace(r.winding == WindMode::CCW ? GL_CCW : GL_CW);
		r0.winding = r.winding;
	}

	if (r0.fill != r.fill) {
		glPolygonMode(GL_FRONT_AND_BACK, r.fill == FillMode::FILL ? GL_FILL : GL_LINE);
		r0.fill = r.fill;
	}

	if (b0.writeMask != b.writeMask) {
		glColorMask(u8(b.writeMask) & 1, u8(b.writeMask) & 2, u8(b.writeMask) & 4, u8(b.writeMask) & 8);
		b0.writeMask = b.writeMask;
	}

	//MSAA

	if (m.samples && m.minSampleShading) {

		if (!ctx.enableMinSampleShading) {
			glEnable(GL_SAMPLE_SHADING);
			ctx.enableMinSampleShading = true;
		}

		if (ctx.minSampleShading != m.minSampleShading) {
			glMinSampleShading(m.minSampleShading);
			ctx.minSampleShading = m.minSampleShading;
		}

	} else if (ctx.enableMinSampleShading) {
		glDisable(GL_SAMPLE_SHADING);
		ctx.enableMinSampleShading = false;
	}

	//Blending

	if (b0.blendEnable != b.blendEnable) {

		if (b.blendEnable) glEnable(GL_BLEND);
		else glDisable(GL_BLEND);

		b0.blendEnable = b.blendEnable;
	}

	if(b0.blendEnable){

		if (b0.blendFactor != b.blendFactor) {
			glBlendColor(b.blendFactor[0], b.blendFactor[1], b.blendFactor[2], b.blendFactor[3]);
			b0.blendFactor = b.blendFactor;
		}

		if (b0.logicOp != b.logicOp) {
			glLogicOp(glxLogicOp(b.logicOp));
			b0.logicOp = b.logicOp;
		}

		if (b0.alphaBlendOp != b.alphaBlendOp || b0.blendOp != b.blendOp) {
			glBlendEquationSeparate(glxBlendOp(b.blendOp), glxBlendOp(b.alphaBlendOp));
			b0.alphaBlendOp = b.alphaBlendOp;
			b0.blendOp = b.blendOp;
		}

		if(
			b0.srcBlend != b.srcBlend || b0.dstBlend != b.dstBlend ||
			b0.alphaSrcBlend != b.alphaSrcBlend || b0.alphaDstBlend != b.alphaDstBlend
		){
			glBlendFuncSeparate(
				glxBlend(b.srcBlend), glxBlend(b.dstBlend),
				glxBlend(b.alphaSrcBlend), glxBlend(b.alphaDstBlend)
			);

			b0.srcBlend = b.srcBlend;
			b0.dstBlend = b.dstBlend;
			b0.alphaSrcBlend = b.alphaSrcBlend;
			b0.alphaDstBlend = b.alphaDstBlend;
		}
	}

	//Depth

	if (d0.enableDepthRead != d.enableDepthRead) {

		if (d.enableDepthRead) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);

		d0.enableDepthRead = d.enableDepthRead;
	}

	if (d0.enableDepthWrite != d.enableDepthWrite) {

		if (d.enableDepthWrite) glDepthMask(true);
		else glDepthMask(false);

		d0.enableDepthWrite = d.enableDepthWrite;
	}

	if (d0.enableDepthRead) {
		if (d0.depthCompare != d.depthCompare)
			glDepthFunc(glxCompareFunc(d0.depthCompare = d.depthCompare));
	}

	//Stencil

	if (d0.enableStencilTest != d.enableStencilTest) {

		if (d.enableStencilTest) glEnable(GL_STENCIL_TEST);
		else glDisable(GL_STENCIL_TEST);

		d0.enableStencilTest = d.enableStencilTest;
	}

	if (d0.enableStencilTest) {

		if (d0.stencilWriteMask != d.stencilWriteMask)
			glStencilMask(d0.stencilMask = d.stencilMask);

		bool update = d0.stencilMask != d.stencilMask || d0.stencilReference != d.stencilReference;

		if (update ||d0.front.compare != d.front.compare)
			glStencilFuncSeparate(
				GL_FRONT, 
				glxCompareFunc(d0.front.compare = d.front.compare),
				d0.stencilReference = d.stencilReference,
				d0.stencilMask = d.stencilMask
			);

		if (update || d0.back.compare != d.back.compare)
			glStencilFuncSeparate(
				GL_BACK,
				glxCompareFunc(d0.back.compare = d.back.compare),
				d0.stencilReference,
				d0.stencilMask
			);

		if (d0.front.fail != d.front.fail || d0.front.pass != d.front.pass ||
			d0.front.depthFail != d.front.depthFail
		)
			glStencilOpSeparate(
				GL_FRONT,
				glxStencilOp(d0.front.fail = d.front.fail),
				glxStencilOp(d0.front.depthFail = d.front.depthFail),
				glxStencilOp(d0.front.pass = d.front.pass)
			);

		if (d0.back.fail != d.back.fail || d0.back.pass != d.back.pass ||
			d0.back.depthFail != d.back.depthFail
		)
			glStencilOpSeparate(
				GL_FRONT,
				glxStencilOp(d0.back.fail = d.back.fail),
				glxStencilOp(d0.back.depthFail = d.back.depthFail),
				glxStencilOp(d0.back.pass = d.back.pass)
			);

	}
}

void glxBindDescriptors(GLContext &ctx, Descriptors *descriptors) {

	if (!descriptors || !descriptors->getInfo().pipelineLayout) return;

	for (auto &mapIt : descriptors->getInfo().pipelineLayout->getInfo()) {

		auto resource = mapIt.second;
		auto it = descriptors->getInfo().flushedResources.find(resource.globalId);

		if (it != descriptors->getInfo().flushedResources.end()) {

			auto &subres = it->second;
			auto *res = subres.resource;

			//TODO: Handle nullptr!

			TextureObject *tex = dynamic_cast<TextureObject*>(res);

			//Bind buffer range

			if (GPUBuffer *buffer = dynamic_cast<GPUBuffer*>(res)) {

				usz offset = subres.bufferRange.offset, size = subres.bufferRange.size;

				GLenum bindPoint = resource.type == ResourceType::CBUFFER ? GL_UNIFORM_BUFFER : GL_SHADER_STORAGE_BUFFER;
				auto &bound = ctx.boundByBaseId[(u64(resource.localId) << 32) | bindPoint];

				if (bound.id == buffer->getId() && bound.offset == offset && bound.size == size)
					continue;

				glBindBufferRange(
					bindPoint, resource.localId, buffer->getData()->handle, offset, size
				);

				bound = { buffer->getId(), offset, size };
			}

			//Bind sampler range

			else if (Sampler *sampler = dynamic_cast<Sampler*>(res)) {

				auto &bound = ctx.boundByBaseId[(u64(resource.localId) << 32) | GL_SAMPLER];

				if (bound.id != sampler->getId()) {
					glBindSampler(resource.localId, sampler->getData()->handle);
					bound.id = sampler->getId();
				}

				tex = subres.samplerData.texture;
			}

			//Bind texture

			if (tex) {

				auto &textureViews = tex->getData()->textureViews;
				GLuint textureView{};

				u32 subId{};

				for (auto &view : textureViews)
					if (view.first == subres.textureRange) {
						textureView = view.second;
						break;
					}
					else ++subId;

				if (!textureView) {

					glGenTextures(1, &textureView);
					glTextureView(
						textureView,
						glxTextureType(subres.textureRange.subType),
						tex->getData()->handle,
						glxColorFormat(tex->getInfo().format),
						subres.textureRange.minLevel,
						subres.textureRange.levelCount,
						subres.textureRange.minLayer,
						subres.textureRange.layerCount
					);

					String name = 
							tex->getName() + " " + 
							std::to_string(textureViews.size());

					glObjectLabel(
						GL_TEXTURE, textureView, GLsizei(name.size()), name.c_str()
					);

					textureViews.push_back({ subres.textureRange, textureView });
				}

				if (!resource.isWritable) {

					auto &boundTex = ctx.boundByBaseId[(u64(resource.localId) << 32) | GL_TEXTURE];

					if (boundTex.id != tex->getId() || boundTex.subId != subId) {
						glBindTextureUnit(resource.localId, textureView);
						boundTex.subId = subId;
						boundTex.id = tex->getId();
					}

				} else {

					auto &boundImg = ctx.boundByBaseId[(u64(resource.localId) << 32) | GL_IMAGE_2D /* Not 2D but GL_IMAGE doesn't exist*/];

					if (boundImg.id != tex->getId() || boundImg.subId != subId) {

						glBindImageTexture(
							resource.localId, textureView, 0,
							GL_TRUE, 0, GL_WRITE_ONLY,
							glxColorFormat(tex->getInfo().format)
						);

						boundImg.subId = subId;
						boundImg.id = tex->getId();
					}
				}
			}
		}
	}
}

//Per context

GLuint glxGenerateVao(PrimitiveBuffer *prim) {
	
	GLuint handle;

	glCreateVertexArrays(1, &handle);
	glObjectLabel(
		GL_VERTEX_ARRAY, handle,
		GLsizei(prim->getName().size()), prim->getName().c_str()
	);

	auto &info = prim->getInfo();

	u32 i{};

	for (auto &v : info.vertexLayout) {

		if (!v.buffer || v.bufferOffset >= v.buffer->size()) {
			oic::System::log()->error("Primitive buffer with null vertex buffer isn't allowed");
			glDeleteVertexArrays(1, &handle);
			return 0;
		}

		glVertexArrayVertexBuffer(
			handle, i, v.buffer->getData()->handle, v.bufferOffset, v.stride()
		);

		for (auto &elem : v.formats) {

			glEnableVertexArrayAttrib(handle, elem.index);
			glVertexArrayAttribFormat(
				handle,
				elem.index, 
				GLint(FormatHelper::getChannelCount(elem.format)),
				glxGpuFormatType(elem.format),
				!FormatHelper::isUnnormalized(elem.format), 
				elem.offset
			);

			glVertexArrayAttribBinding(handle, elem.index, i);

			if (v.instanced())
				glVertexArrayBindingDivisor(handle, elem.index, 1);
		}

		++i;
	}

	if (prim->hasIndices()) {

		if (!info.indexLayout.buffer || info.indexLayout.bufferOffset >= info.indexLayout.buffer->size()) {
			oic::System::log()->error("Primitive buffer with null index buffer isn't allowed");
			glDeleteVertexArrays(1, &handle);
			return 0;
		}

		switch (prim->getIndexFormat().value) {

			case GPUFormat::R16u:
			case GPUFormat::R32u:
				break;

			default:
				oic::System::log()->error("Primitive buffer with invalid index format isn't allowed");
				glDeleteVertexArrays(1, &handle);
				return 0;

		}

		glVertexArrayElementBuffer(
			handle, info.indexLayout.buffer->getData()->handle
		);
	}

	return handle;
}