#pragma once
#include "types/types.hpp"
#define __impl

namespace ignis {

	//A feature that's dependent on the current platform
	//E.g. compute, vr
	enum class Feature : u32 { COMPUTE, COUNT };

	//An extension that's dependent on the current gpu
	//E.g. multi draw indirect
	enum class Extension : u32 { COUNT };

	using Features = Bitset<usz(Feature::COUNT)>;
	using Extensions = Bitset<usz(Extension::COUNT)>;

	enum CommandOp : u32;

	enum class CommandAvailability : u32 {
		SUPPORTED,
		PERFORMANCE,
		UNSUPPORTED
	};

	class GraphicsObject;
	class CommandList;

	class Graphics {

		friend class GraphicsObject;

	public:

		Graphics(const Graphics &) = delete;
		Graphics(Graphics &&) = delete;
		Graphics &operator=(const Graphics &) = delete;
		Graphics &operator=(Graphics &&) = delete;

		const Features &getFeatures() const;
		const Extensions &getExtensions() const;

		const bool hasFeature(Feature) const;
		const bool hasExtension(Extension) const;

		inline auto find(GraphicsObject *t) const;
		inline bool contains(GraphicsObject *t) const;

		GraphicsObject* const *begin() const;
		GraphicsObject* const *end() const;

		template<typename ...args>
		inline void execute(args *...arg) {
			List<CommandList*> commands{ arg... };
			execute(commands);
		}

		__impl Graphics();
		__impl ~Graphics();

		__impl struct Data;

		__impl CommandAvailability getCommandAvailability(CommandOp op);
		__impl void execute(const List<CommandList*> &commands);

		inline Data *getData() { return data; }

	protected:

		inline void erase(GraphicsObject *t);
		inline void add(GraphicsObject *t);
		void clean();

	private:

		Features features;
		Extensions extensions;
		List<GraphicsObject*> graphicsObjects;
		Data *data;

	};
	
	inline auto Graphics::find(GraphicsObject *t) const {
		return std::find(
			graphicsObjects.begin(), graphicsObjects.end(), ( GraphicsObject *) t
		);
	}
	
	inline bool Graphics::contains(GraphicsObject *t) const {
		return find(t) != graphicsObjects.end();
	}
	
	inline void Graphics::add(GraphicsObject *t) {
		if(!contains(t))
			graphicsObjects.push_back(t);
	}

	inline void Graphics::erase(GraphicsObject *t) {

		auto it = find(t);

		if (it != graphicsObjects.end())
			graphicsObjects.erase(it);
	}

}