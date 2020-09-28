#include "graphics/graphics.hpp"
#include "graphics/memory/swapchain.hpp"

namespace ignis {

	GPUObjectId GPUObject::counter = {};

	GPUObject::GPUObject(Graphics &g, const String &name, const GPUObjectType type): 
		name(name), id(counter.newId(type, &g))
	{
		oicAssert("GPUObject with undefined type isn't allowed", type != GPUObjectType::UNDEFINED);
		g.add(this);
	}

	void GPUObject::erase() {
		getGraphics().erase(this);
	}

	bool Graphics::hasFeature(Feature f) const { return features[usz(f)]; }
	bool Graphics::hasExtension(Extension e) const { return extensions[usz(e)]; }

	void Graphics::setFeature(Feature f, bool b) {
		features[usz(f)] = b;
	}

	void Graphics::setExtension(Extension e, bool b) {
		extensions[usz(e)] = b;
	}

	void Graphics::add(GPUObject *t) {

		oicAssert("Graphics::add isn't allowed on a suspended graphics thread", enabledThreads[oic::Thread::getCurrentId()].enabled);
		oicAssert("Couldn't add object with name; it already exists", !contains(t->getName()));

		graphicsObjectsByName[t->getName()] = t;
		graphicsObjects[t->getId()] = t;
	}

	void Graphics::erase(GPUObject *t) {

		auto &thread = enabledThreads[oic::Thread::getCurrentId()];

		oicAssert("Graphics::erase isn't allowed on a suspended graphics thread", thread.enabled);

		auto it = find(t->getName());
		auto itt = find(t->getId());

		if (it != graphicsObjectsByName.end()) {

			oicAssert("Graphics::erase requires graphicsObject to have a valid id and name", itt != graphicsObjects.end());

			graphicsObjectsByName.erase(it);
			graphicsObjects.erase(itt);

			for (auto &elem : enabledThreads)
				elem.second.deleted.push_back(t->getId());
		}
	}

}