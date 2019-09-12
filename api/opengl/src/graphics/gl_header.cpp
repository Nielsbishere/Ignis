#include "graphics/gl_graphics.hpp"

HashMap<String, void**> ignis::glFunctionNames = HashMap<String, void**>();

template<typename T>
static T appendGlFunc(const String &s, T &t) {
	ignis::glFunctionNames[s] = (void**)&t;
	return nullptr;
}

#define GL_FUNC(x, y) PFN##y##PROC x = appendGlFunc(#x, x)

#include "graphics/gl_functions.hpp"