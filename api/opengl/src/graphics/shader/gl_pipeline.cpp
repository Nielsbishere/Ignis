#include "graphics/shader/pipeline.hpp"

namespace ignis {

	Pipeline::Pipeline(Graphics &g, const String &name, const Info &info): GraphicsObject(g, name), info(info) { }
	Pipeline::~Pipeline() { }
}