#pragma once

#include "../tnac_common/pch.h"
#include "../tnac_front/pch.h"
#include "../tnac_eval/pch.h"
#include "../tnac_lib/pch.h"
#include "utils/utils.hpp"

#define TNAC_BAD_PLATFORM static_assert(false, "Platform not supported");

#if _WIN64
#define TNAC_WINDOWS 1
#else
#endif

namespace tnac::rt
{
  using in_stream  = utils::istream;
  using out_stream = utils::ostream;
}