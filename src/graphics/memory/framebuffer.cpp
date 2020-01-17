#include "graphics/memory/framebuffer.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace ignis {

	Framebuffer::Info::Info(
		const Vec2u16 &size, const List<GPUFormat> &colorFormats,
		DepthFormat depthFormat, bool keepDepth, u8 samples
	) :
		colorFormats(colorFormats), size(size),
		depthFormat(depthFormat), keepDepth(keepDepth),
		samples(samples), isDynamic(!size[0] || !size[1])
	{
		if (colorFormats.size() > 8)
			oic::System::log()->fatal("The surface had too many color formats and isn't supported");
	}

}