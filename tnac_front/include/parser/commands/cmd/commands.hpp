//
// Commands
//

#pragma once
#include "parser/ast/ast_util.hpp"

namespace tnac::commands::detail
{
  template <typename F>
  concept cmd_handler = std::is_nothrow_invocable_r_v<void, F, ast::command>;
}

namespace tnac::commands
{
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
  // Verification result
  //
  struct ver_result
  {
    using size_type = ast::command::size_type;

    explicit operator bool() const noexcept
    {
      return m_res == verification::Correct;
    }

    size_type m_expectedArgs{};
    size_type m_diff{};
    verification m_res{ verification::Correct };
  };

  //
  // Command descriptor
  //
  class descr final
  {
  public:
    using param_list = std::vector<tok_kind>;
    using size_type  = param_list::size_type;

    using cmd_handler_t = std::function<void(ast::command)>;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(descr);

    ~descr() noexcept = default;

    template <detail::cmd_handler F>
    descr(param_list params, size_type req, F&& handler) noexcept :
      m_params{ std::move(params) },
      m_reqCount{ req },
      m_cmdHandler{ std::forward<F>(handler) }
    {}

    template <detail::cmd_handler F>
    descr(param_list params, F&& handler) noexcept :
      descr{ std::move(params), size_type{}, std::forward<F>(handler) }
    {
      m_reqCount = m_params.size();
    }

    template <detail::cmd_handler F>
    descr(F&& handler) noexcept :
      descr{ param_list{}, size_type{}, std::forward<F>(handler) }
    {}

  public:
    void operator()(ast::command cmd) const noexcept
    {
      m_cmdHandler(std::move(cmd));
    }

    auto size() const noexcept
    {
      return m_params.size();
    }

    auto required() const noexcept
    {
      return m_reqCount;
    }

    auto operator[](size_type idx) const noexcept
    {
      return m_params[idx];
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
  class store final
  {
  public:
    using name_type  = string_t;
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
    template <typename ...Args>
    void declare(name_type name, Args&& ...args) noexcept
    {
      m_cmds.insert_or_assign(name, cmd_descr{ std::forward<Args>(args)... });
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