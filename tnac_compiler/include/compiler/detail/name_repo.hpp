//
// Name repository
//

#pragma once
#include "cfg/ir/ir_function.hpp"

namespace tnac::detail
{
  //
  // Generates and interns names for various kinds of entities
  //
  class name_repo final
  {
  public:
    //
    // RAII wrapper for indicies cached from the underlying pool
    //
    class idx_cache final
    {
    public:
      using pool_t      = utils::prefixed_pool;
      using cached_data = pool_t::idx_cache;

    public:
      CLASS_SPECIALS_NONE(idx_cache);

      ~idx_cache() noexcept
      {
        m_pool->restore(std::move(m_cache));
      }

      idx_cache(pool_t& pool) noexcept :
        m_pool{ &pool },
        m_cache{ pool.cache_and_reset() }
      {}

    private:
      pool_t* m_pool{};
      cached_data m_cache;
    };

    using param_count = ir::function::size_type;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(name_repo);

    ~name_repo() noexcept;

    name_repo() noexcept;

  public:
    //
    // Returns an index cache
    // This will reset the current indicies for the underlying prefixed pool
    // and restore them in the dtor
    //
    idx_cache init_indicies() noexcept;

    //
    // Returns a name for the entry block of a function
    //
    string_t entry_block_name() noexcept;

    //
    // Returns a name for the ret block of a function
    //
    string_t ret_block_name() noexcept;

    //
    // Creates a mangled name of a module
    //
    string_t mangle_module_name(string_t original, std::size_t parCnt) noexcept;

    //
    // Creates a mangled name of a function
    //
    string_t mangle_func_name(string_t original, const ir::function& owner, std::size_t parCnt) noexcept;

  private:
    //
    // Mangles a function or module name
    //
    string_t mangle(string_t original, const ir::function* owner, std::size_t parCnt) noexcept;

  private:
    utils::string_pool m_plainNames;
    utils::prefixed_pool m_prefNames;
  };
}