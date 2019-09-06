#pragma once
#include "types/types.hpp"

namespace ignis::errors {

	namespace commands {


		static constexpr c8 insufficientData[] = "The command contained insufficient data";
		static constexpr c8 tooBig[] = "The command contained too much data";
		static constexpr c8 outOfBounds[] = "The command list couldn't hold the commands";
		static constexpr c8 lossOfData[] = "The command list resize would result into loss of data";

	}
}