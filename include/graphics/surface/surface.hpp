#pragma once
#include "types/types.hpp"
#include "graphics/graphics_object.hpp"

namespace ignis {

	enum class GPUFormat : u16;
	enum class DepthFormat : u8;
	enum class TextureType : u8;

	class Surface : public GraphicsObject {

	public:

		struct Info {

			Vec2u size;						//The size of the surface
			List<GPUFormat> colorFormats;	//The color formats (<= 8)

			DepthFormat depthFormat;		//The depth format (DepthFormat::NONE or DepthFormat::AUTO generally)
			bool keepDepth;					//If the depth should be maintained after the surface is unbound

			u32 samples;					//If using multisampling; normally set to 1

			bool isDynamic{};				//If true; changes size based on resolution
			f64 viewportScale = 1;			//The scale relative to current resolution

			//Static surface; has to call resize after init
			Info(
				const Vec2u &size, const List<GPUFormat> &colorFormats,
				DepthFormat depthFormat, bool keepDepth, u32 samples = 1
			);

			//Dynamic surface; has to call resize after init
			Info(
				const List<GPUFormat> &colorFormats,
				DepthFormat depthFormat, bool keepDepth,
				u32 samples = 1, f64 viewportScale = 1
			);
		};

		Surface(Graphics &g, const String &name, const Info &info): 
			GraphicsObject(g, name), info(info) {}

		~Surface() = default;

		//Call whenever the screen is resized
		virtual void onResize(const Vec2u &size) = 0;

		virtual void begin(const Vec4u &xywh) = 0;
		virtual void end() = 0;

		virtual bool isGPUWritable() const;
		TextureType getTextureType() const;

		inline const Info &getInfo() const { return info; }

	protected:

		Info info;
	};

}