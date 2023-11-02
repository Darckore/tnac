#pragma once

// std headers we'll most likely be using
#include <type_traits>
#include <concepts>
#include <utility>
#include <source_location>

#include <iostream>
#include <fstream>

#include <string>
#include <string_view>

#include <complex>

#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <stack>
#include <queue>
#include <bitset>

#include <optional>
#include <variant>

#include <filesystem>
#include <memory>
#include <new>

#include <format>

#include <exception>

// just because it is often needed everywhere
using namespace std::literals;
namespace fsys = std::filesystem;

#include "utils/utils.hpp"

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;
}