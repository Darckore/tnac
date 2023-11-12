//
// Core
//

#pragma once
#include "common/feedback.hpp"
#include "parser/parser.hpp"
#include "src_mgr/source_manager.hpp"
#include "sema/sema.hpp"
#include "parser/commands/cmd_interpreter.hpp"

namespace tnac
{
  //
  // Core of the entire tnac library
  //
  class core final
  {
  public:
    using fname_t  = source_manager::path_t;
    using load_res = source_manager::load_res;
    
    using cmd_name_t = commands::store::name_type;

  public:
    CLASS_SPECIALS_NONE(core);

    ~core() noexcept;

    explicit core(feedback& fb) noexcept;

  public:
    //
    // Loads the specified source file
    //
    load_res load(fname_t fname) noexcept;

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
    // Returns the parsed ast
    //
    ast::node* get_ast() noexcept;

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

  private:
    feedback* m_feedback{};
    sema m_sema;
    ast::builder m_astBuilder;
    parser m_parser;

    source_manager m_srcMgr;

    commands::store m_cmdStore;
    cmd m_cmdInterpreter;
  };
}