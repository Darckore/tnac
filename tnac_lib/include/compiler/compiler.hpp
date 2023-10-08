//
// Compiler
//

#pragma once
#include "compiler/src_mgr/source_manager.hpp"
#include "parser/parser.hpp"
#include "parser/ast/ast_builder.hpp"
#include "sema/sema.hpp"

namespace tnac
{
  //
  // Compiles an input file
  //
  class compiler final
  {
  public:
    using filename_t = string_t;
    using input_t    = src::file::contents_t;
    using ast_t      = parser::root_ptr;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(compiler);

    ~compiler() noexcept;
    compiler() noexcept;

  public:
    //
    // Accepts a path to a source file for compilation
    //
    void compile(filename_t fname) noexcept;

  private:
    //
    // Loads a file and returns its contents
    //
    input_t load_file(filename_t fname) noexcept;

    //
    // Parses data obtained from a file
    //
    ast_t parse(string_t input) noexcept;

  private:
    source_manager m_srcMgr;
    ast::builder m_astBuilder;
    sema m_sema;
  };
}