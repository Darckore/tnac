//
// Core
//

#pragma once
#include "common/feedback.hpp"
#include "parser/parser.hpp"
#include "src_mgr/source_manager.hpp"
#include "sema/sema.hpp"

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
    // Parses the entire input file and returns the root ast node
    //
    ast::node* parse(src::file& file) noexcept;

  private:
    feedback* m_feedback{};
    sema m_sema;
    ast::builder m_astBuilder;
    parser m_parser;

    source_manager m_srcMgr;
  };
}