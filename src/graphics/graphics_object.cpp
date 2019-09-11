#include "graphics/graphics_object.hpp"
#include "graphics/graphics.hpp"

namespace ignis {

	GraphicsObject::GraphicsObject(Graphics &g): g(g) {
		g.add(this);
	}

	GraphicsObject::~GraphicsObject() {
		g.erase(this);
	}

}