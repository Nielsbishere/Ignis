#pragma once
#include "graphics_object.hpp"
#include "enums.hpp"

namespace ignis {

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
		D3D12	= 0b0011,
		GPUWEB	= 0b0101,
		METAL	= 0b1001
	};

	class GraphicsObject;
	class CommandList;
	class Framebuffer;
	class Swapchain;

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

		template<typename ...args>
		inline void present(Framebuffer *intermediate, Swapchain *swapchain, args *...arg) {
			List<CommandList*> commands{ arg... };
			present(intermediate, swapchain, commands);
		}

		apimpl Graphics();
		apimpl ~Graphics();

		apimpl GraphicsApi getCurrentApi() const;

		apimpl struct Data;

		apimpl CommandAvailability getCommandAvailability(CommandOp op);
		apimpl void execute(const List<CommandList*> &commands);

		apimpl void present(
			Framebuffer *intermediate, Swapchain *swapchain, 
			const List<CommandList*> &commands
		);

		//Signals that the graphics instance of this thread is not needed currently
		//This enables other threads from sharing with the creator thread
		plimpl void pause();

		//Signals that the graphics instance of this thread is needed currently
		//This blocks other threads from sharing with the creator thread
		plimpl void resume();

		inline Data *getData() { return data; }

	protected:

		plimpl void init();
		plimpl void release();

		inline void erase(GraphicsObject *t);
		inline void add(GraphicsObject *t);
		void clean();

		void setFeature(Feature, bool);
		void setExtension(Extension, bool);

		//API erasion and deletion of objects
		apimpl void onAddOrErase(GraphicsObject *t, bool isDeleted);

	private:

		Features features;
		Extensions extensions;
		List<GraphicsObject*> graphicsObjects;
		Data *data;

	};
	
	inline auto Graphics::find(GraphicsObject *t) const {
		return std::find(
			graphicsObjects.begin(), graphicsObjects.end(), (GraphicsObject*) t
		);
	}
	
	inline bool Graphics::contains(GraphicsObject *t) const {
		return find(t) != graphicsObjects.end();
	}
	
	inline void Graphics::add(GraphicsObject *t) {

		if (!contains(t)) {
			graphicsObjects.push_back(t);
			onAddOrErase(t, false);
		}
	}

	inline void Graphics::erase(GraphicsObject *t) {

		auto it = find(t);

		if (it != graphicsObjects.end()) {
			onAddOrErase(*it, true);
			graphicsObjects.erase(it);
		}
	}

}