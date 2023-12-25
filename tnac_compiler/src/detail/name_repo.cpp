#include "compiler/detail/name_repo.hpp"

namespace tnac::detail
{
  // Special members

  name_repo::~name_repo() noexcept = default;

  name_repo::name_repo() noexcept = default;


  // Public members

  string_t name_repo::entry_block_name() noexcept
  {
    return "entry"sv;
  }

  string_t name_repo::ret_block_name() noexcept
  {
    return "ret"sv;
  }
}