#pragma once

#include "../tnac_lib/pch.h"
#include "utils/utils.hpp"

#include <iostream>
#include <fstream>

#define TNAC_BAD_PLATFORM static_assert(false, "Platform not supported");

#if _WIN64
#define TNAC_WINDOWS 1
#else
#endif

namespace tnac_rt
{
  using in_stream  = std::istream;
  using out_stream = std::ostream;
}