#include "graphics/shader/gl_pipeline.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "utils/hash.hpp"

namespace ignis {

	Pipeline::Pipeline(Graphics &g, const String &name, const Info &info): GraphicsObject(g, name), info(info) { 

		data = new Data();
		data->handles.resize(info.passes.size());

		if (info.passes.size() != 1)
			oic::System::log()->fatal("Multi pass isn't allowed yet");

		for (usz i = 0, j = info.passes.size(); i < j; ++i) {

			GLuint handle = data->handles[i] = glCreateProgram();
			auto &pass = info.passes[i];

			List<GLuint> shaders(pass.size());

			usz k{};

			for (auto &stage : pass) {

				GLenum type = glShaderStage(stage.first);
				GLuint shader = shaders[k] = glCreateShader(type);

				if ((u8(stage.first) & u8(ShaderStage::PROPERTY_IS_TECHNIQUE)) && !g.hasFeature(Feature::MESH_SHADERS))
					oic::System::log()->fatal("Driver doesn't support mesh shaders");	//TODO: Mesh shaders

				if((u8(stage.first) & u8(ShaderStage::PROPERTY_IS_COMPUTE)) && pass.size() != 1)
					oic::System::log()->fatal("Can't create a shader with mixed compute and graphics stages");

				String shaderName = NAME(name + " " + std::to_string(i) + " shader " + std::to_string(u8(stage.first)));
				glObjectLabel(GL_SHADER, shader, GLsizei(shaderName.size()), shaderName.c_str());
				glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, stage.second.data(), GLsizei(stage.second.size()));
				glSpecializeShader(shader, "main", 0, nullptr, nullptr);

				String error;

				if (glCheckShaderLog(shader, error))
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

			if (glCheckProgramLog(handle, error))
				oic::System::log()->fatal("Couldn't link shader");

		}
	}

	Pipeline::~Pipeline() { 

		for(auto &handle : data->handles)
			glDeleteProgram(handle);

		delete data;
	}
}