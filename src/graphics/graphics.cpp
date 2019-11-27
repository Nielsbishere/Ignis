#include "graphics/graphics.hpp"
#include "graphics/graphics_object.hpp"

namespace ignis {

	const Features &Graphics::getFeatures() const { return features; }
	const Extensions &Graphics::getExtensions() const { return extensions; }

	const bool Graphics::hasFeature(Feature f) const { return features[usz(f)]; }
	const bool Graphics::hasExtension(Extension e) const { return extensions[usz(e)]; }

	void Graphics::setFeature(Feature f, bool b) {
		features[usz(f)] = b;
	}

	void Graphics::setExtension(Extension e, bool b) {
		extensions[usz(e)] = b;
	}
}