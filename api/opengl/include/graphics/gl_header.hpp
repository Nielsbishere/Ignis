#pragma once

#define GL_FUNC(x, y) extern PFN##y##PROC x;
#include "graphics/gl_functions.hpp"
#undef GL_FUNC

namespace ignis {
	extern HashMap<String, void**> glFunctionNames;
}