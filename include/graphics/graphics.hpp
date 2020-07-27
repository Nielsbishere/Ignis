#pragma once
#define apimpl /* this function is api dependent */
#define plimpl /* this function is platform and/or api dependent */
#include "enums.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "utils/thread.hpp"
#include "utils/hash.hpp"
#include "types/vec.hpp"

namespace ignis {

	using Features = Bitset<usz(Feature::COUNT)>;
	using Extensions = Bitset<usz(Extension::COUNT)>;

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
		WEBGPU	= 0b0101,
		METAL	= 0b1001
	};

	//Each type has to be unique, and as such if the last bits are equal, the id would have to increment
	//& 1 = hasGpuMemory (if it has a considerable potential gpu memory overhead)
	//& 2 = isTexture
	//& 4 = isRenderTarget
	//& 8 = isContainer (something that holds a resource; hasGpuMemory is generally false)
	//& 16 = isBuffer
	//& 32 = isResource
	//>> 8 = id
	enum class GPUObjectType : u64 {

		UNDEFINED = 0x000,
		PIPELINE_LAYOUT = 0x100,

		PIPELINE = 0x001,
		COMMAND_LIST = 0x101,
		DESCRIPTORS = 0x201,

		FRAMEBUFFER = 0x008,
		PRIMITIVE_BUFFER = 0x108,
		SWAPCHAIN = 0x208,
		UPLOAD_BUFFER = 0x308,

		SAMPLER = 0x020,

		TEXTURE = 0x023,

		DEPTH_TEXTURE = 0x027,
		RENDER_TEXTURE = 0x127,

		SHADER_BUFFER = 0x028,

		BUFFER = 0x031,

		PROPERTY_HAS_GPU_MEMORY = 1 << 0,
		PROPERTY_IS_TEXTURE = 1 << 1,
		PROPERTY_IS_RENDER_TEXTURE = 1 << 2,
		PROPERTY_IS_CONTAINER = 1 << 3,
		PROPERTY_IS_BUFFER = 1 << 4,
		PROPERTY_IS_RESOURCE = 1 << 5,

		PROPERTY_UNUSED = 0xC0,

		PROPERTY_FLAGS = 0xFF,

		PROPERTY_RSHIFT_ID = 8

	};

	//Type mappings

	class Graphics;
	class GPUObject;
	class Pipeline;
	class CommandList;
	class Descriptors;
	class Framebuffer;
	class PrimitiveBuffer;
	class Swapchain;
	class Sampler;
	class Texture;
	class DepthTexture;
	class RenderTexture;
	class ShaderBuffer;
	class GPUBuffer;
	class PipelineLayout;
	class UploadBuffer;

	template<typename T> static constexpr GPUObjectType asTypeId = GPUObjectType::UNDEFINED;
	template<> static constexpr GPUObjectType asTypeId<Pipeline> = GPUObjectType::PIPELINE;
	template<> static constexpr GPUObjectType asTypeId<CommandList> = GPUObjectType::COMMAND_LIST;
	template<> static constexpr GPUObjectType asTypeId<Descriptors> = GPUObjectType::DESCRIPTORS;
	template<> static constexpr GPUObjectType asTypeId<Framebuffer> = GPUObjectType::FRAMEBUFFER;
	template<> static constexpr GPUObjectType asTypeId<PrimitiveBuffer> = GPUObjectType::PRIMITIVE_BUFFER;
	template<> static constexpr GPUObjectType asTypeId<Swapchain> = GPUObjectType::SWAPCHAIN;
	template<> static constexpr GPUObjectType asTypeId<Sampler> = GPUObjectType::SAMPLER;
	template<> static constexpr GPUObjectType asTypeId<Texture> = GPUObjectType::TEXTURE;
	template<> static constexpr GPUObjectType asTypeId<DepthTexture> = GPUObjectType::DEPTH_TEXTURE;
	template<> static constexpr GPUObjectType asTypeId<RenderTexture> = GPUObjectType::RENDER_TEXTURE;
	template<> static constexpr GPUObjectType asTypeId<ShaderBuffer> = GPUObjectType::SHADER_BUFFER;
	template<> static constexpr GPUObjectType asTypeId<GPUBuffer> = GPUObjectType::BUFFER;
	template<> static constexpr GPUObjectType asTypeId<PipelineLayout> = GPUObjectType::PIPELINE_LAYOUT;
	template<> static constexpr GPUObjectType asTypeId<UploadBuffer> = GPUObjectType::UPLOAD_BUFFER;

	//Wrapper around a GPUObject that is unique, even after the object's lifetime

	struct GPUObjectId {

		u64 lo, hi;
		GPUObjectType type;
		Graphics *g;

		GPUObjectId(u64 hi, u64 lo, GPUObjectType t, Graphics *g): hi(hi), lo(lo), g(g), type(t) {}
		GPUObjectId(): GPUObjectId(0, 0, GPUObjectType::UNDEFINED, nullptr) {}

		inline GPUObjectId &operator++() {
			if (!(++lo)) ++hi;
			return *this;
		}

		inline GPUObjectId newId(GPUObjectType t, Graphics *gr) {
			++*this;
			return GPUObjectId(lo, hi, t, gr);
		}

		inline bool null() const { return !lo && !hi; }
		inline bool operator==(const GPUObjectId &other) const { return lo == other.lo && hi == other.hi; }
		inline bool operator!=(const GPUObjectId &other) const { return !operator==(other); }

		template<typename T>
		inline T *get() const;

		//True if the resource stopped existing
		inline bool vanished() const { return !null() && !get<GPUObject>(); }

	};

	//Only use with T base of GPUObject
	template<typename T>
	static GPUObjectId getGPUObjectId(const T *t) {

		if (GPUObject *g = (GPUObject *)t)
			return g->getId();

		return GPUObjectId{};
	}

}

namespace std {

	template<>
	struct hash<ignis::GPUObjectId> {

		usz operator()(const ignis::GPUObjectId &rid) const {

			u64 hash64 = oic::Hash::hash64(rid.lo, rid.hi);

			if constexpr (sizeof(usz) != sizeof(u64))
				return oic::Hash::hash32(u32(hash64 >> 32), u32(hash64 & u32_MAX));

			return hash64;
		}

	};

}

namespace ignis {

	class TextureObject;
	class UploadBuffer;

	class Graphics {

		friend class GPUObject;

	public:

		using PresentToCpuCallback = void (*)(void*, UploadBuffer*, const Pair<u64, u64>&, TextureObject*, const Vec3u16&, const Vec3u16&, u16, u8, bool);

		Graphics() = delete;
		Graphics(const Graphics &) = delete;
		Graphics(Graphics &&) = delete;
		Graphics &operator=(const Graphics &) = delete;
		Graphics &operator=(Graphics &&) = delete;

		const Features &getFeatures() const;
		const Extensions &getExtensions() const;

		bool hasFeature(Feature) const;
		bool hasExtension(Extension) const;

		inline auto find(const String &name) const;
		inline bool contains(const String &name) const;

		inline auto find(const GPUObjectId &id) const;
		inline bool contains(const GPUObjectId &id) const;

		inline auto begin() { return graphicsObjects.begin(); }
		inline auto begin() const { return graphicsObjects.begin(); }

		inline auto end() { return graphicsObjects.end(); }
		inline auto end() const { return graphicsObjects.end(); }

		inline auto beginByName() { return graphicsObjectsByName.begin(); }
		inline auto beginByName() const { return graphicsObjectsByName.begin(); }

		inline auto endByName() { return graphicsObjectsByName.end(); }
		inline auto endByName() const { return graphicsObjectsByName.end(); }

		template<typename ...args>
		inline void execute(const args &...arg) {
			List<CommandList*> commands{ arg... };
			execute(commands);
		}

		template<typename ...args>
		inline void present(Framebuffer *intermediate, Swapchain *swapchain, args ...arg) {
			List<CommandList*> commands{ arg... };
			present(intermediate, swapchain, commands);
		}

		apimpl Graphics(
			const String &applicationName,
			const u32 applicationVersion,
			const String &engineName,
			const u32 engineVersion
		) throw();

		template<typename ...args>
		inline void present(Texture *intermediate, u16 slice, u16 mip, Swapchain *swapchain, args ...arg) {
			List<CommandList*> commands{ arg... };
			present(intermediate, slice, mip, swapchain, commands);
		}

		apimpl ~Graphics();

		apimpl GraphicsApi getCurrentApi() const;

		apimpl struct Data;

		apimpl void execute(const List<CommandList*> &commands);

		apimpl void present(
			Framebuffer *intermediate, Swapchain *swapchain, 
			const List<CommandList*> &commands
		);

		apimpl void present(
			Texture *intermediate, u16 slice, u16 mip, Swapchain *swapchain, 
			const List<CommandList*> &commands
		);

		template<typename T, void (T::*func)(UploadBuffer*, const Pair<u64, u64>&, TextureObject*, const Vec3u16&, const Vec3u16&, u16, u8, bool)>
		inline void presentToCpu(

			const List<CommandList*> &commands,
			TextureObject *target,
			UploadBuffer *result,

			T *callbackInstance,

			Vec3u16 size = {}, Vec3u16 offset = {},
			u8 mip = 0,
			u16 layer = 0,

			bool isStencil = false
		) {
			oicAssert("Upload buffer, TextureObject and Commands required", result && target && commands.size());
			oicAssert("Callback and callbackInstance required", func && callbackInstance);

			presentToCpuInternal(commands, target, result, (PresentToCpuCallback) &func, callbackInstance, size, offset, mip, layer, isStencil);
		}

		//Wait until the GPU has executed pending instructions from this thread
		apimpl void wait();

		//Signals that the graphics instance of this thread is not needed currently
		//This enables other threads from sharing with the creator thread
		//Must be called when a thread is stopped or doesn't call the graphics anymore
		apimpl plimpl void pause();

		//Signals that the graphics instance of this thread is needed currently
		//This blocks other threads from sharing with the creator thread
		//Required to be called if a thread starts to call graphics
		apimpl plimpl void resume();

		inline Data *getData() const { return data; }

		const String appName, engineName;
		const u32 appVersion, engineVersion;

		inline String name() {
			return oic::Log::concat(
				appName, " v", appVersion,
				" (", engineName, " v", engineVersion + ")"
			);
		}

		//Check if this thread is able to do GPU calls
		inline bool isThreadEnabled() {
			return enabledThreads[oic::Thread::getCurrentId()].enabled;
		}

		//Get the deleted objects for this thread
		inline auto &getDeletedObjects() {
			return enabledThreads[oic::Thread::getCurrentId()].deleted;
		}

	protected:

		//isIndepedentExecution specifies if this was called directly by "execute"
		//or if an internal function will handle the syncing & resource tracking, etc.
		//
		apimpl List<GPUObject*> executeInternal(const List<CommandList*> &commands, bool isIndepedentExecution);

		apimpl void presentToCpuInternal(
			const List<CommandList *> &commands,
			TextureObject *target,
			UploadBuffer *result,
			PresentToCpuCallback callback,
			void *callbackInstance,
			Vec3u16 size, Vec3u16 offset,
			u8 mip,
			u16 layer,
			bool isStencil
		);

		plimpl void init();
		plimpl void release();

		void erase(GPUObject *t);
		void add(GPUObject *t);

		void setFeature(Feature, bool);
		void setExtension(Extension, bool);

	private:

		Features features;
		Extensions extensions;

		HashMap<String, GPUObject*> graphicsObjectsByName;
		HashMap<GPUObjectId, GPUObject*> graphicsObjects;

		struct GraphicsThread { 
			List<GPUObjectId> deleted;
			bool enabled{};
		};

		HashMap<usz, GraphicsThread> enabledThreads;

		Data *data;

	};

	//GPU object

	class GPUObject {

	public:

		//Creates a GraphicsObject with one reference
		GPUObject(Graphics &g, const String &name, const GPUObjectType type);

		GPUObject(const GPUObject&) = delete;
		GPUObject(GPUObject&&) = delete;
		GPUObject &operator=(const GPUObject&) = delete;
		GPUObject &operator=(GPUObject&&) = delete;

		template<GPUObjectType t>
		inline bool canCast() const { return id.type == t; }

		template<typename T>
		inline T *cast() { return canCast<asTypeId<T>>() ? (T*)this : nullptr; }

		inline const String &getName() const { return name; }
		inline const GPUObjectId &getId() const { return id; }
		inline const GPUObjectType getType() const { return id.type; }

		//When a ref is added; it will have to be removed or the resource will be left over
		inline void addRef() { refCount++; }

		//Lose a reference; only way to destruct the object
		inline void loseRef() {
			if (!(--refCount)) {
				erase();
				delete this;
			}
		}

		inline Graphics &getGraphics() const { return *id.g; }

	protected:

		virtual ~GPUObject() {}

	private:

		void erase();

		GPUObjectId id;

		String name;
		u64 refCount = 1;

		static GPUObjectId counter;

	};

	//A wrapper for constructing and destructing graphics objects
	template<typename T>
	class GraphicsObjectRef {

		T *ptr{};

	public:

		using Ptr = T*;

		void release() {
			if (ptr) {
				ptr->loseRef();
				ptr = nullptr;
			}
		}

		GraphicsObjectRef() { 
			static_assert(std::is_base_of_v<GPUObject, T>, "GraphicsObjectRef can only be used on GraphicsObjects");
		}

		~GraphicsObjectRef() {
			release();
		}

		GraphicsObjectRef(Ptr ptr): ptr(ptr) {
			if(ptr)
				ptr->addRef();
		}

		GraphicsObjectRef(const GraphicsObjectRef &other) {
			if((ptr = other.ptr) != nullptr)
				ptr->addRef();
		}

		GraphicsObjectRef(GraphicsObjectRef &&other) {
			ptr = other.ptr;
			other.ptr = nullptr;
		}

		GraphicsObjectRef &operator=(const GraphicsObjectRef &other) {

			release();

			if((ptr = other.ptr) != nullptr)
				ptr->addRef();

			return *this;
		}

		//Maintain equal references (the other one
		GraphicsObjectRef &operator=(GraphicsObjectRef &&other) noexcept {
			ptr = other.ptr;
			other.ptr = nullptr;
			return *this;
		}

		inline T *operator->() { return ptr; }
		inline T *operator->() const { return ptr; }

		inline operator T*() const { return ptr; }
		inline operator T*() { return ptr; }

		//Creates a graphics object
		GraphicsObjectRef(Graphics &g, const String &name, const typename T::Info &info) {

			//Find existing resource

			auto it = g.find(name);

			oicAssert("The requested resource already exists", it == g.endByName());

			//Try and create resource (nullptr if it fails)

			try {
				ptr = new T(g, name, info);
			} catch (std::runtime_error&) { }
		}

		//Finds a GraphicsObject
		GraphicsObjectRef(Graphics &g, const String &name) {

			//Find existing resource

			auto it = g.find(name);

			if (it != g.end()) {
				ptr = it->second;
				ptr->addRef();
			}
		}

		inline bool exists() const { return ptr; }
		inline bool null() const { return !ptr; }

		//Use this if you can't use a wrapped object and need the actual pointer
		inline T *get() const { return ptr; }
	};

	//Definitions
	
	inline auto Graphics::find(const String &name) const {
		return graphicsObjectsByName.find(name);
	}

	inline bool Graphics::contains(const String &name) const {
		return find(name) != graphicsObjectsByName.end();
	}

	inline auto Graphics::find(const GPUObjectId &id) const {
		return graphicsObjects.find(id);
	}

	inline bool Graphics::contains(const GPUObjectId &id) const {
		return find(id) != graphicsObjects.end();
	}

	template<typename T>
	inline T *GPUObjectId::get() const {

		static_assert(std::is_base_of_v<GPUObject, T>, "GPUObjectId::operator T* requires T to be a GPUObject");

		if (!g)
			return nullptr;

		if(constexpr(!std::is_same_v<T, GPUObject>))
			oicAssert("Incompatible resource types", asTypeId<T> == type);

		auto it = g->find(*this);

		if(it == g->end())
			return nullptr;

		return (T*) it->second;
	}

}

namespace std {

	template<typename T>
	struct hash<ignis::GraphicsObjectRef<T>> {

		inline usz operator()(const ignis::GraphicsObjectRef<T> &t) const {
			return usz(t.get());
		}

	};
}