#pragma once
#include "types/vec.hpp"

#define GL_FUNC(x, y) extern PFN##y##PROC x;
#include "graphics/gl_functions.hpp"
#undef GL_FUNC

namespace ignis {

	extern HashMap<String, void**> glFunctionNames;

	enum class DepthFormat : u8;
	enum class GPUBufferType : u8;
	enum class GPUMemoryUsage : u8;
	class GPUFormat;
	enum class GPUFormatType : u8;
	enum class TextureType : u8;
	enum class TopologyMode : u8;
	enum class ShaderStage : u8;
	enum class SamplerMode : u8;
	enum class SamplerMag : u8;
	enum class SamplerMin : u8;
	enum class CompareOp : u8;
	enum class StencilOp : u8;

	class Descriptors;
	class Pipeline;
	class PrimitiveBuffer;

	struct GLContext;

	namespace cmd { struct SetClearColor; }
}

extern GLenum glxDepthFormat(ignis::DepthFormat format);
extern GLenum glxColorFormat(ignis::GPUFormat format);
extern GLenum glxBufferType(ignis::GPUBufferType format);
extern GLenum glxGpuFormatType(ignis::GPUFormat type);
extern GLenum glxGpuDataFormat(ignis::GPUFormat type);
extern GLenum glxTopologyMode(ignis::TopologyMode topo);
extern GLenum glxShaderStage(ignis::ShaderStage stage);
extern GLenum glxTextureType(ignis::TextureType type);
extern GLenum glxSamplerMode(ignis::SamplerMode mode);
extern GLenum glxSamplerMag(ignis::SamplerMag mag);
extern GLenum glxSamplerMin(ignis::SamplerMin min);
extern GLenum glxCompareFunc(ignis::CompareOp compareOp);
extern GLenum glxStencilOp(ignis::StencilOp stencilOp);

extern void glxBeginRenderPass(
	ignis::GLContext &data, const ignis::GPUObjectId &framebuffer, GLuint i
);

extern void glxClearFramebuffer(ignis::GLContext& ctx, GLuint fbo, GLuint index, const ignis::cmd::SetClearColor& clearColor);

extern void glxSetViewport(ignis::GLContext &data, const Vec2u32 &size, const Vec2i32 &offset);

extern void APIENTRY glxDebugMessage(
	GLenum, GLenum, GLuint, GLenum,
	GLsizei, const GLchar*, const void*
);

extern void glxBindPipeline(ignis::GLContext &data, ignis::Pipeline *pipeline);
extern void glxBindDescriptors(ignis::GLContext &data, const List<ignis::Descriptors*> &descriptors);
extern bool glxCheckShaderLog(GLuint shader, String &str);
extern bool glxCheckProgramLog(GLuint program, String &str);

//Per context objects

extern GLuint glxGenerateVao(ignis::PrimitiveBuffer *prim);