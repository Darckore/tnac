//
// Core package for the entire tnac library
//

#pragma once
#include "packages/pkg/parser_pkg.hpp"
#include "packages/pkg/evaluator_pkg.hpp"
#include "packages/pkg/cmd_pkg.hpp"

namespace tnac::packages
{
  //
  // The core package
  //
  class tnac_core final
  {
  public:
    using size_type = evaluator::size_type;

  public:
    CLASS_SPECIALS_NONE(tnac_core);

    ~tnac_core() noexcept;

    explicit tnac_core(size_type stackSize) noexcept;

  public:
    parser& get_parser() noexcept;
    evaluator& get_eval() noexcept;
    cmd& get_commands() noexcept;

  private:
    parser m_parser;
    evaluator m_ev;
    cmd m_cmd;
  };

}