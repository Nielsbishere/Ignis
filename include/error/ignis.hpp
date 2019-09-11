#pragma once
#include "types/types.hpp"

namespace ignis::errors {

	namespace commands {

		static constexpr c8 tooBig[] = "The command contained too much data";
		static constexpr c8 outOfBounds[] = "The command list couldn't hold the commands";
		static constexpr c8 lossOfData[] = "The command list resize would result into loss of data";
		static constexpr c8 performanceLoss[] = "The command is not natively supported and could lead to performance loss";
		static constexpr c8 notSupported[] = "The command is not supported and will result in undefined behavior";
	}

	namespace surface {

		static constexpr c8 tooManyFormats[] = "The surface had too many color formats and isn't supported";
		static constexpr c8 contextError[] = "The graphics context couldn't be created";
	}
}