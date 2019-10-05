#pragma once
#include "../graphics_object.hpp"
#include "graphics/memory/buffer_layout.hpp"

namespace ignis {

	enum class TopologyMode : u8;
	enum class FillMode : u8;
	enum class WindMode : u8;
	enum class CullMode : u8;
	enum class ShaderStage : u8;

	class Pipeline : public GraphicsObject {

	public:

		enum class Flag : u8 {
			OPTIMIZE = 1 << 0
		};

		struct Rasterizer {

			FillMode fill;
			CullMode cull;
			WindMode winding;

			Rasterizer(FillMode fill, CullMode cull, WindMode wind): 
				fill(fill), cull(cull), winding(wind) {}

		};

		struct Info {

			List<BufferAttributes> attributeLayout;
			List<HashMap<ShaderStage, Buffer>> passes;

			Flag flag;
			TopologyMode topology;
			Rasterizer rasterizer;

			Info(
				Flag f, const List<BufferAttributes> &attributeLayout, TopologyMode topology, 
				const HashMap<ShaderStage, Buffer> &passes, Rasterizer rasterizer
			) : 
				flag(f), attributeLayout(attributeLayout), topology(topology), passes{passes}, rasterizer(rasterizer) { }

			Info(
				Flag f, const List<BufferAttributes> &attributeLayout, TopologyMode topology, 
				const List<HashMap<ShaderStage, Buffer>> &passes, Rasterizer rasterizer
			) : 
				flag(f), attributeLayout(attributeLayout), topology(topology), passes(passes), rasterizer(rasterizer) { }

		};

		apimpl struct Data;

		apimpl Pipeline(Graphics &g, const String &name, const Info &info);
		apimpl ~Pipeline();

		const Info &getInfo() const { return info; }
		Data *getData() { return data; }

	private:

		Info info;
		Data *data{};
	};

}