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
	enum class TextureType : u8;
	enum class TopologyMode : u8;
	enum class ShaderStage : u8;

	class Descriptors;
	class Pipeline;
}

extern GLenum glxDepthFormat(ignis::DepthFormat format);
extern GLenum glxColorFormat(ignis::GPUFormat format);
extern GLenum glxBufferType(ignis::GPUBufferType format);
extern GLenum glxBufferUsage(ignis::GPUMemoryUsage usage, bool isPersistent);
extern GLenum glxBufferHint(ignis::GPUMemoryUsage usage);
extern GLenum glxGpuFormatType(ignis::GPUFormat type);
extern GLenum glxGpuDataFormat(ignis::GPUFormat type);
extern GLenum glxTopologyMode(ignis::TopologyMode topo);
extern GLenum glxShaderStage(ignis::ShaderStage stage);
extern GLenum glxTextureType(ignis::TextureType type);

extern void glxBeginRenderPass(
	ignis::Graphics::Data &data, const Vec4u &renderArea, const Vec2u &size, GLuint framebuffer
);

extern void APIENTRY glxDebugMessage(
	GLenum, GLenum, GLuint, GLenum,
	GLsizei, const GLchar*, const void*
);

extern void glxBindPipeline(ignis::Graphics::Data &data, ignis::Pipeline *pipeline);
extern void glxBindDescriptors(ignis::Graphics::Data &data, ignis::Descriptors *descriptors);
extern bool glxCheckShaderLog(GLuint shader, String &str);
extern bool glxCheckProgramLog(GLuint program, String &str);