#include "graphics/graphics.hpp"

namespace ignis {

	Graphics::Graphics(
		SurfaceManager *surfaces, ResourceManager *resources, 
		const Features &features, const Extensions &extensions
	):
		surfaces(surfaces), resources(resources),
		features(features), extensions(extensions) {}

	SurfaceManager *Graphics::getSurfaceManager() { return surfaces; }
	ResourceManager *Graphics::getResourceManager() { return resources; }

	const Features &Graphics::getFeatures() const { return features; }
	const Extensions &Graphics::getExtensions() const { return extensions; }

	const bool Graphics::hasFeature(Feature f) const { return features[usz(f)]; }
	const bool Graphics::hasExtension(Extension e) const { return extensions[usz(e)]; }
}