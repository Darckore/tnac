//
// A convenient way to init everything the parser needs
//

#pragma once
#include "parser/lex.hpp"
#include "parser/parser.hpp"
#include "ast/ast_builder.hpp"
#include "sema/sema.hpp"

namespace tnac::packages
{
  //
  // A wrapper for all things parser
  //
  class parser final
  {
  public:
    using ast_ptr        = tnac::parser::pointer;
    using ast_root       = tnac::parser::root_ptr;
    using const_ast_root = tnac::parser::const_root_ptr;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(parser);

    ~parser() noexcept;

    parser() noexcept;

  public:
    //
    // Redirects to tnac::parser::operator()
    //
    ast_ptr operator()(string_t input) noexcept;

    //
    // Redirects to tnac::parser::root() const
    //
    const_ast_root root() const noexcept;

    //
    // Redirects to tnac::parser::root()
    //
    ast_root root() noexcept;

  public: // callbacks
    //
    // tnac::parser::on_error
    //
    template <detail::err_handler F>
    void on_parse_error(F&& handler) noexcept
    {
      m_parser.on_error(std::forward<F>(handler));
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
      m_sema.on_variable(std::forward<F>(handler));
    }

  private:
    ast::builder m_builder;
    sema m_sema;
    tnac::parser m_parser;
  };
}