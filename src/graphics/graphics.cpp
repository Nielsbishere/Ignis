#include "graphics/graphics.hpp"
#include "graphics/graphics_object.hpp"

namespace ignis {

	const Features &Graphics::getFeatures() const { return features; }
	const Extensions &Graphics::getExtensions() const { return extensions; }

	const bool Graphics::hasFeature(Feature f) const { return features[usz(f)]; }
	const bool Graphics::hasExtension(Extension e) const { return extensions[usz(e)]; }

	GraphicsObject *const *Graphics::begin() const { return graphicsObjects.data(); }
	GraphicsObject *const *Graphics::end() const { return graphicsObjects.data() + graphicsObjects.size(); }

	void Graphics::clean() {

		for (GraphicsObject *go : graphicsObjects)
			delete go;

		graphicsObjects.clear();
	}

	void Graphics::setFeature(Feature f, bool b) {
		features[usz(f)] = b;
	}

	void Graphics::setExtension(Extension e, bool b) {
		extensions[usz(e)] = b;
	}
}