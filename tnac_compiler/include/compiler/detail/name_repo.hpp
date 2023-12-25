//
// Name repository
//

#pragma once

namespace tnac::detail
{
  //
  // Generates and interns names for various kinds of entities
  //
  class name_repo final
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(name_repo);

    ~name_repo() noexcept;

    name_repo() noexcept;

  public:
    //
    // Returns a name for the entry block of a function
    //
    string_t entry_block_name() noexcept;

    //
    // Returns a name for the ret block of a function
    //
    string_t ret_block_name() noexcept;

  private:
    utils::string_pool m_plainNames;
    utils::prefixed_pool m_prefNames;
  };
}