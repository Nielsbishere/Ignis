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

	void Texture::flush(const List<TextureRange> &ranges) {

		for(auto &range : ranges)
			if(!isValidRange(range))
				oic::System::log()->fatal("Texture::flush out of bounds");

		for(auto &range : ranges)
			info.pending.push_back(range);

		mergePending();
	}


	void Texture::mergePending() {

		constexpr i32 
			delta1D = 15,	//If there's a 1D difference of <=15 texels
			delta2D = 7,	//If there's a 2D difference of <=7 texels
			delta3D = 3;	//If there's a 3D difference of <=3 texels

		auto *back = &info.pending.back();

		do {

			auto dim = getDimensions(back->mip);

			const i32 delta = dim.z ? delta3D : (dim.y ? delta2D : delta1D);

			auto *oldBack = back;

			for (auto &pending : info.pending) {

				if (&pending == back || pending.mip != back->mip)
					continue;

				Vec3i32 diffBegin = (pending.start + pending.size - back->start).cast<Vec3i32>();
				Vec3i32 diffEnd = (pending.start - back->start - back->size).cast<Vec3i32>();

				if ((diffBegin >= -delta).all() && (diffEnd <= delta).all()) {

					auto start = pending.start;

					pending.start = pending.start.min(back->start);
					pending.size = (start + pending.size).max(back->start + back->size) - pending.start;

					usz off = &pending - info.pending.data();

					info.pending.erase(info.pending.begin() + (back - info.pending.data()));
					back = info.pending.data() + off;
					break;
				}
			}

			if (oldBack == back || info.pending.size() == 1)
				break;

		} while (true);

	}

}