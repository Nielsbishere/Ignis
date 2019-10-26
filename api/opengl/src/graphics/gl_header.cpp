#include "graphics/enums.hpp"
#include "system/log.hpp"
#include "system/system.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/memory/gl_gpu_buffer.hpp"
#include "graphics/gl_graphics.hpp"

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

GLenum glDepthFormat(DepthFormat format) {

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

GLenum glColorFormat(GPUFormat format){

	switch (format) {

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

		default:
			oic::System::log()->fatal("Invalid depth format");
			return GL_RGBA8;

	}

}

GLenum glBufferType(GPUBufferType format) {

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

GLenum glBufferUsage(GPUBufferUsage usage, bool isPersistent) {

	GLenum res{};

	if (u8(usage) & u8(GPUBufferUsage::CPU_WRITE)) {
		res |= GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;
		if(isPersistent) res |= GL_MAP_PERSISTENT_BIT;
	}

	if (u8(usage) & u8(GPUBufferUsage::SHARED))
		res |= GL_CLIENT_STORAGE_BIT;

	return res;
}

GLenum glBufferHint(GPUBufferUsage usage) {

	//& 1 = isStatic
	//& 2 = isCopy
	constexpr GLenum type[] = {
		GL_DYNAMIC_DRAW, GL_STATIC_DRAW,
		GL_DYNAMIC_COPY, GL_STATIC_COPY
	};

	usz id{};

	if (!(u8(usage) & u8(GPUBufferUsage::CPU_WRITE))) {

		id |= 2;		//If CPU doesn't write; it's draw

		if (!(u8(usage) & u8(GPUBufferUsage::GPU_WRITE)))
			id |= 1;	//If GPU also doesn't write; it's static
	}

	return type[id];
}

GLenum glGpuFormat(GPUFormat type) {

	const GPUFormatType t = FormatHelper::getType(type);
	const usz stride = FormatHelper::getStrideBits(type);
	
	if (t == GPUFormatType::FLOAT) {

		if (stride == 16)
			return GL_HALF_FLOAT;

		else if (stride == 32)
			return GL_FLOAT;

		else goto error;
	}

	const bool isSigned = FormatHelper::isSigned(type);

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

GLenum glTopologyMode(TopologyMode topo) {

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
	}

	oic::System::log()->fatal("Unsupported topology mode");
	return {};
}

GLenum glShaderStage(ShaderStage stage) {

	if (u8(stage) & 0x40)
		oic::System::log()->fatal("OpenGL doesn't natively support raytracing");

	switch (stage) {

		case ignis::ShaderStage::VERTEX:			return GL_VERTEX_SHADER;
		case ignis::ShaderStage::GEOMETRY:			return GL_GEOMETRY_SHADER;
		case ignis::ShaderStage::TESS_CTRL:			return GL_TESS_CONTROL_SHADER;
		case ignis::ShaderStage::TESS_EVAL:			return GL_TESS_EVALUATION_SHADER;
		case ignis::ShaderStage::FRAGMENT:			return GL_FRAGMENT_SHADER;
		case ignis::ShaderStage::COMPUTE:			return GL_COMPUTE_SHADER;
		case ignis::ShaderStage::TASK_EXT:			return GL_TASK_SHADER_NV;
		case ignis::ShaderStage::MESH_EXT:			return GL_MESH_SHADER_NV;
	}

	oic::System::log()->fatal("Invalid shader stage");
	return {};
}

//Functionality

void glBeginRenderPass(
	Graphics::Data &gdata, const Vec4u &xywh, const Vec2u &size, GLuint framebuffer
) {

	if (gdata.bound[GL_READ_FRAMEBUFFER] != framebuffer || gdata.bound[GL_DRAW_FRAMEBUFFER] != framebuffer)
		glBindFramebuffer(GL_FRAMEBUFFER, gdata.bound[GL_READ_FRAMEBUFFER] = gdata.bound[GL_DRAW_FRAMEBUFFER] = framebuffer);

	Vec4u sc = xywh;

	if (!sc[2])
		sc[2] = size[0] - sc[0];

	if (!sc[3])
		sc[3] = size[1] - sc[1];

	Vec4u vp = { 0, 0, size[0] - sc[0], size[1] - sc[1] };

	if (gdata.viewport != vp) {
		gdata.viewport = vp;
		glViewport(vp[0], vp[1], vp[2], vp[3]);
	}

	if (sc == vp) {
		if (gdata.scissorEnable) {
			glDisable(GL_SCISSOR_TEST);
			gdata.scissorEnable = false;
		}
	}
	else if (!gdata.scissorEnable) {
		glEnable(GL_SCISSOR_TEST);
		gdata.scissorEnable = true;
	}

	if (gdata.scissor != sc) {
		gdata.scissor = sc;
		glScissor(sc[0], sc[1], sc[2], sc[3]);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void glDebugMessage(
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

bool glCheckShaderLog(GLuint shader, String &str) {
	return glCheckLog<GL_COMPILE_STATUS>(glGetShaderiv, glGetShaderInfoLog, shader, str);
}

bool glCheckProgramLog(GLuint program, String &str) {
	return glCheckLog<GL_LINK_STATUS>(glGetProgramiv, glGetProgramInfoLog, program, str);
}

void glBindPipeline(Graphics::Data &g, Pipeline *pipeline) {

	glUseProgram(pipeline->getData()->handles[0]);

	auto &r = pipeline->getInfo().rasterizer;

	if (g.cullMode != r.cull) {

		if (u8(g.cullMode) && !u8(r.cull))
			glDisable(GL_CULL_FACE);

		if (!u8(g.cullMode) && u8(r.cull))
			glEnable(GL_CULL_FACE);

		if (u8(r.cull))
			glCullFace(r.cull == CullMode::BACK ? GL_BACK : GL_FRONT);

		g.cullMode = r.cull;
	}

	if (g.windMode != r.winding && u8(r.cull)) {
		glFrontFace(r.winding == WindMode::CCW ? GL_CCW : GL_CW);
		g.windMode = r.winding;
	}

	if (g.fillMode != r.fill) {
		glPolygonMode(GL_FRONT_AND_BACK, r.fill == FillMode::FILL ? GL_FILL : GL_LINE);
		g.fillMode = r.fill;
	}

}

void glBindDescriptors(Graphics::Data &g, Descriptors *descriptors) {

	for (auto &mapIt : descriptors->getInfo().pipelineLayout) {

		auto resource = mapIt.second;
		auto it = descriptors->getInfo().resources.find(resource.globalId);

		if (it != descriptors->getInfo().resources.end()) {

			auto *res = it->second;

			if (GPUBuffer *buffer = res->cast<GPUBuffer>()) {

				GLenum bindPoint = resource.type == ResourceType::CBUFFER ? GL_UNIFORM_BUFFER : GL_SHADER_STORAGE_BUFFER;
				GLuint &bound = g.boundByBase[(u64(resource.localId) << 32) | bindPoint];

				if (bound == buffer->getData()->handle)
					continue;

				glBindBufferBase(bindPoint, resource.localId, bound = buffer->getData()->handle);
			}

			//TODO: Textures, Samplers, etc.

		}
	}

}