#include "driver/input_mgr.hpp"

namespace tnac_rt
{
  // Special members

  input_mgr::~input_mgr() noexcept = default;

  input_mgr::input_mgr() noexcept = default;

  // Public members

  input_mgr::stored_input& input_mgr::input(tnac::buf_t in) noexcept
  {
    return store(std::move(in));
  }

  input_mgr::stored_input* input_mgr::from_file(tnac::string_t fname) noexcept
  {
    fsys::path fn{ fname };
    std::error_code errc;
    fn = fsys::absolute(fn, errc);
    if (errc)
    {
      err() << "Path '" << fn.string() << "' is invalid. " << errc.message() << '\n';
      return {};
    }

    tnac::buf_t buf;
    std::ifstream in{ fn.string() };
    if (!in)
    {
      err() << "Unable to open the input file '" << fn.string() << "'\n";
      return {};
    }

    in.seekg(0, std::ios::end);
    buf.reserve(in.tellg());
    in.seekg(0, std::ios::beg);

    using it = std::istreambuf_iterator<tnac::buf_t::value_type>;
    buf.assign(it{ in }, it{});
    return &store(std::move(buf), std::move(fn));
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