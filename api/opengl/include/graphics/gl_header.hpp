#pragma once

#define GL_FUNC(x, y) extern PFN##y##PROC x;
#include "graphics/gl_functions.hpp"
#undef GL_FUNC

namespace ignis {

	extern HashMap<String, void**> glFunctionNames;

	enum class DepthFormat : u8;
	enum class GPUBufferType : u8;
	enum class GPUMemoryUsage : u8;
	enum class GPUFormat : u16;
	enum class GPUFormatType : u8;
	enum class TopologyMode : u8;
	enum class ShaderStage : u8;

	class Descriptors;
	class Pipeline;
}

extern GLenum glDepthFormat(ignis::DepthFormat format);
extern GLenum glColorFormat(ignis::GPUFormat format);
extern GLenum glBufferType(ignis::GPUBufferType format);
extern GLenum glBufferUsage(ignis::GPUMemoryUsage usage, bool isPersistent);
extern GLenum glBufferHint(ignis::GPUMemoryUsage usage);
extern GLenum glGpuFormat(ignis::GPUFormat type);
extern GLenum glTopologyMode(ignis::TopologyMode topo);
extern GLenum glShaderStage(ignis::ShaderStage stage);

extern void glBeginRenderPass(
	ignis::Graphics::Data &data, const Vec4u &renderArea, const Vec2u &size, GLuint framebuffer
);

extern void APIENTRY glDebugMessage(
	GLenum, GLenum, GLuint, GLenum,
	GLsizei, const GLchar*, const void*
);

extern void glBindPipeline(ignis::Graphics::Data &data, ignis::Pipeline *pipeline);
extern void glBindDescriptors(ignis::Graphics::Data &data, ignis::Descriptors *descriptors);
extern bool glCheckShaderLog(GLuint shader, String &str);
extern bool glCheckProgramLog(GLuint program, String &str);