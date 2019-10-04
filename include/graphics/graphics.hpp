#pragma once
#include "graphics_object.hpp"

namespace ignis {

	//A feature that's dependent on the current platform
	enum class Feature : u32 { COMPUTE, STORAGE_BUFFER, COUNT };

	//An extension that's dependent on the current gpu
	enum class Extension : u32 { DRAW_INDIRECT, DISPATCH_INDIRECT, COUNT };

	using Features = Bitset<usz(Feature::COUNT)>;
	using Extensions = Bitset<usz(Extension::COUNT)>;

	enum CommandOp : u32;

	enum class CommandAvailability : u32 {
		SUPPORTED,
		PERFORMANCE,
		UNSUPPORTED
	};

	//A unique bitset specifying the apis (not necessarily implemented)
	//& 1 = isLowLevel
	//& 2 = isWindows (!isWindows; unix)
	//& 4 = isWeb	  (!isWeb; local app)
	//& 8 = isMac
	//& 0xE = isPlatformSpecific
	enum class GraphicsApi : u8 {

		OPENGL	= 0b0000,
		VULKAN	= 0b0001,
		D3D11	= 0b0010,
		D3D12	= 0b0011,
		WEBGL2	= 0b0100,
		GPUWEB	= 0b0101,
		METAL	= 0b1001
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

		GraphicsObject *const *begin() const;
		GraphicsObject *const *end() const;

		template<typename ...args>
		inline void execute(args *...arg) {
			List<CommandList*> commands{ arg... };
			execute(commands);
		}

		apimpl Graphics();
		apimpl ~Graphics();

		apimpl GraphicsApi getCurrentApi() const;

		plimpl void init();

		apimpl struct Data;

		apimpl CommandAvailability getCommandAvailability(CommandOp op);
		apimpl void execute(const List<CommandList*> &commands);

		inline Data *getData() { return data; }

	protected:

		inline void erase(GraphicsObject *t);
		inline void add(GraphicsObject *t);
		void clean();

		void setFeature(Feature, bool);
		void setExtension(Extension, bool);

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