//
// Commands
//

#pragma once

namespace tnac_rt::commands
{
  namespace detail
  {
    template <typename F>
    concept cmd_handler = std::is_nothrow_invocable_r_v<void, F, tnac::ast::command>;
  }

  //
  // Indicates whether or not a command passes verification
  //
  enum class verification : std::uint8_t
  {
    Correct,
    WrongName,
    TooFew,
    TooMany,
    WrongKind
  };

  //
  // Command descriptor
  //
  class descr
  {
  public:
    using param_list = std::vector<tnac::tok_kind>;
    using size_type  = param_list::size_type;

    using cmd_handler_t = std::function<void(tnac::ast::command)>;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(descr);

    ~descr() noexcept = default;

    template <detail::cmd_handler F>
    descr(F&& handler, param_list params, size_type req) noexcept :
      m_params{ std::move(params) },
      m_reqCount{ req },
      m_cmdHandler{ std::forward<F>(handler) }
    {}

    template <detail::cmd_handler F>
    descr(F&& handler, param_list params) noexcept :
      descr{ std::forward<F>(handler), std::move(params), size_type{} }
    {
      m_reqCount = m_params.size();
    }

    template <detail::cmd_handler F>
    descr(F&& handler) noexcept :
      descr{ std::forward<F>(handler), param_list{}, size_type{} }
    {}

  public:
    const cmd_handler_t& handler() const noexcept
    {
      return m_cmdHandler;
    }


  private:
    param_list m_params;
    size_type m_reqCount{};
    cmd_handler_t m_cmdHandler;
  };


  //
  // Storage for commands
  // This is initied by whoever is interested in defining a custom command set
  //
  class store
  {
  public:
    using name_type  = tnac::string_t;
    using cmd_descr  = descr;
    using descr_ptr  = const cmd_descr*;
    using storage    = std::unordered_map<name_type, cmd_descr>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(store);

    ~store() noexcept = default;

    store() noexcept = default;

  public:
    //
    // Declares a command with the specified name and parameters
    // Subsequent additions overwrite the existing records
    //
    template <detail::cmd_handler F, typename ...Args>
    void declare(name_type name, F&& handler, Args&& ...args) noexcept
    {
      m_cmds.insert_or_assign(name, cmd_descr{ std::forward<F>(handler), std::forward<Args>(args)... });
    }

    //
    // Searches for an entry by name
    //
    descr_ptr find(name_type name) noexcept
    {
      auto iter = m_cmds.find(name);
      return iter != m_cmds.end() ? &iter->second : descr_ptr{};
    }

  private:
    storage m_cmds;
  };
}