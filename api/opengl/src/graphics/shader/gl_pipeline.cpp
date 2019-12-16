#include "graphics/shader/gl_pipeline.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "utils/hash.hpp"

namespace ignis {

	Pipeline::Pipeline(Graphics &g, const String &name, const Info &info): GraphicsObject(g, name), info(info) { 

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

			if((u8(stage.first) & u8(ShaderStage::PROPERTY_IS_COMPUTE)) && info.stages.size() != 1)
				oic::System::log()->fatal("Can't create a shader with mixed compute and graphics stages");

			String shaderName = NAME(name + " shader " + std::to_string(u8(stage.first)));
			glObjectLabel(GL_SHADER, shader, GLsizei(shaderName.size()), shaderName.c_str());
			glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, stage.second.data(), GLsizei(stage.second.size()));
			glSpecializeShader(shader, "main", 0, nullptr, nullptr);

			String error;

			if (glxCheckShaderLog(shader, error))
				oic::System::log()->fatal("Couldn't compile shader");
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
			oic::System::log()->fatal("Couldn't link shader");

	}

	Pipeline::~Pipeline() { 
		glDeleteProgram(data->handle);
		destroy(data);
	}
}