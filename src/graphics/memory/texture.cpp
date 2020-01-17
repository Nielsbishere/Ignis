#include "graphics/memory/texture.hpp"
#include "graphics/shader/descriptors.hpp"
#include "utils/math.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

namespace ignis {

	bool Texture::Info::init(const List<Buffer> &b) {

		if (b.size() != usz(mips)) {
			oic::System::log()->error("Texture requires all init data (mips and layers)");
			return false;
		}

		Vec3u16 res = dimensions;

		for (usz m = 0; m < mips; ++m) {

			usz size =
				FormatHelper::getSizeBytes(format) * 
				res.x * res.y * res.z * layers;

			if (b[m].size() != size || !size) {
				oic::System::log()->error("Invalid texture size");
				return false;
			}

			res = (res.cast<Vec3f64>() / 2).ceil().cast<Vec3u16>();
		}

		initData = b;
		return true;
	}

}