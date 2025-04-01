//
// Core
//

#pragma once
#include "common/feedback.hpp"
#include "parser/parser.hpp"
#include "src_mgr/source_manager.hpp"
#include "sema/sema.hpp"
#include "parser/commands/cmd_interpreter.hpp"
#include "eval/value/value_registry.hpp"
#include "compiler/compiler.hpp"
#include "cfg/cfg.hpp"

namespace tnac
{
  //
  // Core of the entire tnac library
  // Incorporates all components and provides an interface to them
  // Basically, a god object to avoid bothering with individual parts on their own
  //
  class core final
  {
  public:
    using fname_t    = source_manager::path_t;
    using cmd_name_t = commands::store::name_type;

  public:
    CLASS_SPECIALS_NONE(core);

    ~core() noexcept;

    explicit core(feedback& fb) noexcept;

  public:
    //
    // Loads the specified source file
    //
    src::file* load_source(fname_t fname) noexcept;

    //
    // Loads a file by the given path
    // Also, performs parsing and compilation
    //
    bool process_file(fname_t fname) noexcept;

    //
    // Parses the given input and returns the parse result
    //
    ast::node* parse(string_t input) noexcept;

    //
    // Parses the given input using the given location and returns the parse result
    //
    ast::node* parse(string_t input, src::location& loc) noexcept;

    //
    // Parses the entire input file and returns the root ast node
    //
    ast::node* parse(src::file& file) noexcept;

    //
    // Compiles code from an AST node
    //
    void compile(ast::node& node) noexcept;

    //
    // Compiles code from the current AST
    //
    void compile() noexcept;

    //
    // Returns the parsed ast
    //
    const ast::node* get_ast() const noexcept;

    //
    // Returns the parsed ast
    //
    ast::node* get_ast() noexcept;

    //
    // Returns the current CFG
    //
    const ir::cfg& get_cfg() const noexcept;

    //
    // Returns the current CFG
    //
    ir::cfg& get_cfg() noexcept;

  public:
    //
    // Declares a command
    //
    template <typename ...Args>
    void declare_cmd(cmd_name_t name, Args&& ...args) noexcept
    {
      m_cmdStore.declare(name, std::forward<Args>(args)...);
    }

    //
    // Processes a command
    //
    void process_cmd(ast::command cmd) noexcept;

    //
    // Retrieves a line from source file by location
    //
    string_t fetch_line(src::loc_wrapper loc) const noexcept;

    //
    // Returns an iterable collection of all declared variables
    //
    auto variables() const noexcept
    {
      return m_sema.vars();
    }

    //
    // Returns an iterable collection of all declared functions
    //
    auto functions() const noexcept
    {
      return m_sema.funcs();
    }

    // Returns an iterable collection of all declared modules
    //
    auto modules() const noexcept
    {
      return m_sema.modules();
    }

  private:
    // common
    feedback* m_feedback{};

    // front
    sema m_sema;
    ast::builder m_astBuilder;
    parser m_parser;
    source_manager m_srcMgr;
    commands::store m_cmdStore;
    cmd m_cmdInterpreter;

    // compilation
    ir::builder m_irBuilder;
    ir::cfg m_cfg;
    compiler m_compiler;
  };
}