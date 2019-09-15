#pragma once
#include "types/types.hpp"
#include "graphics/resource.hpp"
#define __impl

namespace ignis {

	enum class GPUFormat : u16;
	enum class DepthFormat : u8;

	class Surface : public GPUResource {

	public:

		struct Info {

			Vec2u size;						//The size of the surface
			List<GPUFormat> colorFormats;	//The color formats (<= 8)

			DepthFormat depthFormat;		//The depth format (DepthFormat::NONE or DepthFormat::AUTO generally)
			bool keepDepth;					//If the depth should be maintained after the surface is unbound

			u32 samples;					//If using multisampling; normally set to 1

			Info(
				const Vec2u &size, const List<GPUFormat> &colorFormats,
				DepthFormat depthFormat, bool keepDepth, u32 samples = 1
			);
		};

		Surface(Graphics &g, const String &name, const Info &info): GPUResource(g, name), info(info) {}
		~Surface() = default;

		virtual void onResize(const Vec2u &size) = 0;

		virtual void begin(const Vec4u &xywh) = 0;
		virtual void end() = 0;

		inline const Info &getInfo() const { return info; }

	protected:

		Info info;
	};

}