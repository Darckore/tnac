#include "driver/input_mgr.hpp"

namespace tnac_rt
{
  // Special members

  input_mgr::~input_mgr() noexcept = default;

  input_mgr::input_mgr(src_mgr& mgr) noexcept :
    m_srcMgr{ mgr }
  {}

  // Public members

  input_mgr::stored_input& input_mgr::input(tnac::buf_t in) noexcept
  {
    return store(std::move(in));
  }

  input_mgr::stored_input* input_mgr::from_file(tnac::string_t fname) noexcept
  {
    auto loadRes = m_srcMgr.load(fname);
    if (!loadRes)
    {
      err() << "Unable to load file '" << fname << "': "
            << loadRes.error().message() << '\n';
      return {};
    }

    auto file = *loadRes;
    if (auto fileCont = file->get_contents(); !fileCont)
    {
      err() << "Unable to read data from file '" << fname << "': "
            << loadRes.error().message() << '\n';
      return {};
    }

    return &store(file);
  }

  void input_mgr::on_error(const tnac::token& tok, tnac::string_t msg) noexcept
  {
    utils::unused(tok);
    err() << msg << '\n';
  }

  void input_mgr::on_parse_error(const tnac::ast::error_expr& error) noexcept
  {
    on_error(error.at(), error.message());
  }

  // Private members

  out_stream& input_mgr::err() noexcept
  {
    return *m_err;
  }

}