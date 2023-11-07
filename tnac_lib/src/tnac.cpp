#include "core/tnac.hpp"

namespace tnac
{
  // Special members

  core::~core() noexcept = default;

  core::core(feedback& fb) noexcept :
    m_feedback{ &fb },
    m_parser{ m_astBuilder, m_sema }
  {
    m_parser.attach_feedback(fb);
  }


  // Public members

  core::load_res core::load(fname_t fname) noexcept
  {
    return m_srcMgr.load(std::move(fname));
  }

  ast::node* core::parse(string_t input) noexcept
  {
    return m_parser(input);
  }

  ast::node* core::parse(src::file& file) noexcept
  {
    auto fileContents = file.get_contents();
    if (!fileContents)
    {
      return {};
    }

    auto loc = file.make_location();
    m_parser(*fileContents, loc);
    return m_parser.root();
  }

}