#include "compiler/compiler.hpp"
#include "compiler/cfg/cfg_builder.hpp"

namespace tnac
{
  // Special members

  compiler::~compiler() noexcept = default;

  compiler::compiler() noexcept = default;


  // Public members

  void compiler::compile(filename_t fname) noexcept
  {
    auto inputData = load_file(fname);
    if (!inputData)
      return;

    auto ast = parse(*inputData);
    build_cfg(ast);
  }

  comp::cfg& compiler::cfg() noexcept
  {
    return m_cfg;
  }


  // Private members

  compiler::input_t compiler::load_file(filename_t fname) noexcept
  {
    auto inputFile = m_srcMgr.load(fname);
    if (!inputFile)
      return std::unexpected{ inputFile.error() };

    return (*inputFile)->get_contents();
  }

  compiler::ast_t compiler::parse(string_t input) noexcept
  {
    parser p{ m_astBuilder, m_sema };
    p(input);
    return p.root();
  }

  void compiler::build_cfg(ast_t entry) noexcept
  {
    comp::cfg_builder cb{ m_cfg };
    cb(entry);
  }

}