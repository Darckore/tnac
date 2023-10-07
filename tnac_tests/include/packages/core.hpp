//
// Core package for the entire tnac library
//

#pragma once
#include "packages/pkg/parser_pkg.hpp"
#include "packages/pkg/evaluator_pkg.hpp"
#include "packages/pkg/cmd_pkg.hpp"
#include "packages/pkg/compiler_pkg.hpp"

namespace tnac::packages
{
  //
  // The core package
  //
  class tnac_core final
  {
  public:
    using size_type   = evaluator::size_type;
    using name_type   = cmd::name_type;
    using result_type = eval::value;

  public:
    CLASS_SPECIALS_NONE(tnac_core);

    ~tnac_core() noexcept;

    explicit tnac_core(size_type stackSize) noexcept;

  public:
    parser& get_parser() noexcept;
    evaluator& get_eval() noexcept;
    compiler& get_compiler() noexcept;
    cmd& get_commands() noexcept;

  public:
    //
    // Redirects to tnac::commands::store::declare
    //
    template <typename ...Args>
    void declare_command(name_type name, Args&& ...args) noexcept
    {
      m_cmd.declare(name, std::forward<Args>(args)...);
    }

    //
    // Parses the input and produces a result
    //
    result_type evaluate(string_t input) noexcept;


  public: // callbacks
    //
    // tnac::cmd::on_error
    //
    template <commands::detail::cmd_err_handler F>
    void on_command_error(F&& handler) noexcept
    {
      m_cmd.on_error(std::forward<F>(handler));
    }

    //
    // tnac::evaluator::on_error
    //
    template <eval::detail::err_handler F>
    void on_semantic_error(F&& handler) noexcept
    {
      m_ev.on_error(std::forward<F>(handler));
    }

    //
    // tnac::parser::on_error
    //
    template <detail::parse_err_handler F>
    void on_parse_error(F&& handler) noexcept
    {
      m_parser.on_parse_error(std::forward<F>(handler));
    }

    //
    // tnac::parser::on_command
    //
    template <detail::cmd_handler F>
    void on_command(F&& handler) noexcept
    {
      m_parser.on_command(std::forward<F>(handler));
    }

    //
    // sema::on_variable
    //
    template <detail::var_handler F>
    void on_variable_declaration(F&& handler) noexcept
    {
      m_parser.on_variable_declaration(std::forward<F>(handler));
    }

  private:
    parser m_parser;
    evaluator m_ev;
    compiler m_compiler;
    cmd m_cmd;
  };

}