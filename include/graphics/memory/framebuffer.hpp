#pragma once
#include "../graphics_object.hpp"
#include "../enums.hpp"
#include "types/vec.hpp"

namespace ignis {

	class RenderTexture;
	class DepthTexture;
	
	class Framebuffer : public GraphicsObject {

	public:

		struct Info {

			List<GPUFormat> colorFormats;		//The color formats (<= 8)

			Vec2u16 size;						//The size of the surface

			f64 viewportScale = 1;				//The scale relative to current resolution

			DepthFormat depthFormat;			//The depth format (DepthFormat::NONE or DepthFormat::AUTO generally)
			bool keepDepth;						//If the depth should be maintained after the surface is unbound

			u8 samples;							//If using multisampling; normally set to 1

			bool isDynamic{};					//If true; changes size based on resolution

			//Static surface; has to call resize after init
			Info(
				const Vec2u16 &size, const List<GPUFormat> &colorFormats,
				DepthFormat depthFormat, bool keepDepth, u8 samples = 1
			);

			//Dynamic surface; has to call resize after init
			Info(
				const List<GPUFormat> &colorFormats,
				DepthFormat depthFormat, bool keepDepth,
				u8 samples = 1, f64 scale = 1
			) :
				Info(Vec2u16(), colorFormats, depthFormat, keepDepth, samples)
			{
				viewportScale = scale;
			}
		};

		apimpl struct Data;
		apimpl Framebuffer(Graphics &g, const String &name, const Info &info);

		apimpl void onResize(const Vec2u32 &size);

		apimpl void begin();
		apimpl void end();

		inline const Info &getInfo() const { return info; }
		inline Data *getData() const { return data; }

		inline usz size() const { return targets.size(); }
		inline RenderTexture *getTarget(usz i) const { return targets[i]; }

		inline DepthTexture *getDepth() const { return depth; }

	private:

		apimpl ~Framebuffer();

		List<RenderTexture*> targets;
		DepthTexture *depth{};
		Data *data;

		Info info;
	};

}