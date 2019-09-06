#pragma once
#include "types/types.hpp"

namespace ignis {

	//A feature that's dependent on the current platform
	//E.g. compute, vr
	enum class Feature : u32 { COMPUTE, COUNT };

	//An extension that's dependent on the current gpu
	//E.g. multi draw indirect
	enum class Extension : u32 { COUNT };

	using Features = Bitset<usz(Feature::COUNT)>;
	using Extensions = Bitset<usz(Extension::COUNT)>;

	enum class CommandOp : u32;

	enum class CommandAvailability : u32 {
		SUPPORTED,
		PERFORMANCE,
		UNSUPPORTED
	};

	class SurfaceManager;
	class ResourceManager;

	class Graphics {

	public:
		
		Graphics(SurfaceManager*, ResourceManager*, const Features&, const Extensions&);
		virtual ~Graphics() = default;

		Graphics(const Graphics&) = delete;
		Graphics(Graphics&&) = delete;
		Graphics &operator=(const Graphics&) = delete;
		Graphics &operator=(Graphics&&) = delete;

		SurfaceManager *getSurfaceManager();
		ResourceManager *getResourceManager();

		const Features &getFeatures() const;
		const Extensions &getExtensions() const;

		const bool hasFeature(Feature) const;
		const bool hasExtension(Extension) const;

		virtual CommandAvailability getCommandAvailability(CommandOp op) = 0;

	private:

		Features features;
		Extensions extensions;
		SurfaceManager *surfaces;
		ResourceManager *resources;

	};

}