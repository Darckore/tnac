#pragma once
#include <complex>
#include "utils/utils.hpp"

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;

  using file_data = std::expected<string_t, std::error_code>;
}


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