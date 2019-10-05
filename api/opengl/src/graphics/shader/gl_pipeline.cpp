#include "graphics/shader/gl_pipeline.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

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

				GLuint shader = shaders[k] = glCreateShader(glShaderStage(stage.first));

				if ((u8(stage.first) & 0x80) && !g.hasFeature(Feature::MESH_SHADERS))
					oic::System::log()->fatal("Driver doesn't support mesh shaders");	//TODO: Mesh shaders

				if((u8(stage.first) & 0x20) && pass.size() != 1)
					oic::System::log()->fatal("Can't create a shader with mixed compute and graphics stages");

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