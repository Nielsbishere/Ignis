#include "error/ignis.hpp"
#include "graphics/surface/surface.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {

	Surface::Info::Info(
		const Vec2u &size, const List<GPUFormat> &colorFormats,
		DepthFormat depthFormat, bool keepDepth
	):
		size(size), colorFormats(colorFormats),
		depthFormat(depthFormat), keepDepth(keepDepth)
	{

		if (colorFormats.size() > 8)
			oic::System::log()->warn(errors::surface::tooManyFormats);
	}

}