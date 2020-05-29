#include "graphics/shader/gl_pipeline.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "utils/hash.hpp"

namespace ignis {

	PipelineLayout::PipelineLayout(Graphics &g, const String &name, const Info &inf) :
		GPUObject(g, name, GPUObjectType::PIPELINE_LAYOUT), info(inf) {}

	PipelineLayout::~PipelineLayout() {}

	Pipeline::Pipeline(Graphics &g, const String &name, const Info &info):
		GPUObject(g, name, GPUObjectType::PIPELINE), info(info) { 

		data = new Data();

		GLuint handle = data->handle = glCreateProgram();

		glObjectLabel(
			GL_PROGRAM, handle, GLsizei(name.size()), name.c_str()
		);

		List<GLuint> shaders(info.stages.size());

		usz k{};

		for (auto &stage : info.stages) {

			GLenum type = glxShaderStage(stage.first);
			GLuint shader = shaders[k] = glCreateShader(type);

			if ((u8(stage.first) & u8(ShaderStage::PROPERTY_IS_TECHNIQUE)) && !g.hasFeature(Feature::MESH_SHADERS))
				oic::System::log()->fatal("Driver doesn't support mesh shaders");

			const Buffer &bin = info.binaries[stage.second.first];

			oicAssert("SPIR-V has to be 4-byte aligned", !(bin.size() & 3));

			glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, bin.data(), GLsizei(bin.size()));
			glSpecializeShader(shader, stage.second.second.c_str(), 0, nullptr, nullptr);

			String error;

			if (glxCheckShaderLog(shader, error))
				oic::System::log()->fatal("Couldn't compile shader ", error);
			else
				glAttachShader(handle, shader);

			++k;
		}

		glLinkProgram(handle);

		for (GLuint shader : shaders) {
			glDetachShader(handle, shader);
			glDeleteShader(shader);
		}

		String error;

		if (glxCheckProgramLog(handle, error))
			oic::System::log()->fatal("Couldn't link shader ", error);

	}

	Pipeline::~Pipeline() { 
		glDeleteProgram(data->handle);
		destroy(data);
	}
}