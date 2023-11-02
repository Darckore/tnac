//
// Compiler
//

#pragma once
#include "src_mgr/source_manager.hpp"
#include "parser/parser.hpp"
#include "parser/ast/ast_builder.hpp"
#include "sema/sema.hpp"
#include "compiler/cfg/cfg.hpp"

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

    //
    // Returns a reference to the program's cfg
    //
    comp::cfg& cfg() noexcept;

  private:
    //
    // Loads a file and returns its contents
    //
    input_t load_file(filename_t fname) noexcept;

    //
    // Parses data obtained from a file
    //
    ast_t parse(string_t input) noexcept;

    //
    // Builds control-flow graph
    //
    void build_cfg(ast_t entry) noexcept;

    //
    // Registers the current module
    //
    void register_module() noexcept;

  private:
    source_manager m_srcMgr;
    src::file* m_src{};
    ast::builder m_astBuilder;
    sema m_sema;
    comp::cfg m_cfg;
  };
}