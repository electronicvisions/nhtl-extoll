#pragma once

#include <sstream>

#include "rma2.h"

namespace nhtl_extoll {

template <typename E, typename... Args>
void throw_on_error(RMA2_ERROR status, Args... args)
{
	if (status != RMA2_SUCCESS) {
		throw E(args...);
	}
}

}
