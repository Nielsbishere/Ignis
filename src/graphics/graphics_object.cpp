#include "graphics/graphics_object.hpp"
#include "graphics/graphics.hpp"

namespace ignis {

	GraphicsObject::GraphicsObject(Graphics &g, const String &name): g(g), name(name) {
		g.add(this);
	}

	void GraphicsObject::erase() {
		g.erase(this);
	}

}