#include "graphics/graphics_object.hpp"
#include "graphics/graphics.hpp"

namespace ignis {

	GraphicsObject::GraphicsObject(Graphics &g, const String &name): g(g), name(name) {
		g.add(this);
	}

	GraphicsObject::~GraphicsObject() {

		if (refCount > 1)
			oic::System::log()->fatal("The graphics object was still being referenced");

		g.erase(this);
	}

}