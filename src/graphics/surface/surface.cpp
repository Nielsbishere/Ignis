#include "error/ignis.hpp"
#include "graphics/surface/surface.hpp"
#include "graphics/shader/pipeline_layout.hpp"
#include "graphics/enums.hpp"
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
		u32 samples, f64 scale
	): 
		Info(Vec2u(), colorFormats, depthFormat, keepDepth, samples)
	{
		viewportScale = scale;
	}

	bool Surface::isCompatible(
		const RegisterLayout &reg, const GPUSubresource &
	) const {
		return
			reg.type		 == ResourceType::TEXTURE && 
			reg.textureType	 == getTextureType() && 
			(!reg.isWritable || isGPUWritable());
	}

	bool Surface::isGPUWritable() const {
		return false;
	}

	TextureType Surface::getTextureType() const {
		return info.samples > 1 ? TextureType::TEXTURE_MS : TextureType::TEXTURE_2D;
	}

}