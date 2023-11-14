#pragma once

#include "utils/utils.hpp"

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;

  using file_data = std::expected<string_t, std::error_code>;
}