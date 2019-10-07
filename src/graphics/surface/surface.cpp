#include "error/ignis.hpp"
#include "graphics/surface/surface.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {

	Surface::Info::Info(
		const Vec2u &size, const List<GPUFormat> &colorFormats,
		DepthFormat depthFormat, bool keepDepth, u32 samples
	):
		size(size), colorFormats(colorFormats),
		depthFormat(depthFormat), keepDepth(keepDepth), samples(samples),
		isDynamic(size[0] == 0 || size[1] == 0)
	{

		if (colorFormats.size() > 8)
			oic::System::log()->warn(errors::surface::tooManyFormats);
	}

	Surface::Info::Info(
		const List<GPUFormat> &colorFormats,
		DepthFormat depthFormat, bool keepDepth,
		f64 scale, u32 samples
	): 
		Info(Vec2u(), colorFormats, depthFormat, keepDepth, samples)
	{
		viewportScale = scale;
	}

}