//
// Definitions common to all modules
//

#pragma once

#define TNAC_BAD_PLATFORM static_assert(false, "Platform not supported");

#if _WIN64
#define TNAC_WINDOWS 1
#else
#endif

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;
}