#include "driver/source_manager.hpp"

namespace tnac_rt
{
  // Special members

  src_manager::~src_manager() noexcept = default;

  src_manager::src_manager() noexcept = default;

  // Public members

  src_manager::stored_input& src_manager::input(tnac::buf_t in) noexcept
  {
    m_prevIdx = m_inputIdx;
    auto newItem = m_input.emplace(m_inputIdx++, std::move(in));
    return newItem.first->second;
  }

  src_manager::stored_input* src_manager::from_file(tnac::string_t fname) noexcept
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
    return &input(std::move(buf));
  }

  void src_manager::on_error(const tnac::token& tok, tnac::string_t msg) noexcept
  {
    utils::unused(tok);
    err() << msg << '\n';
  }

  void src_manager::on_parse_error(const tnac::ast::error_expr& error) noexcept
  {
    on_error(error.at(), error.message());
  }

  // Private members

  out_stream& src_manager::err() noexcept
  {
    return *m_err;
  }

}