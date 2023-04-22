#include "driver/driver.hpp"
#include "output/printer.hpp"
#include "output/lister.hpp"

namespace tnac_rt
{
  // Special members

  driver::driver() noexcept :
    m_parser{ m_builder, m_sema },
    m_cmd{ m_commands }
  {
    m_cmd.on_error([this](auto&& tok, auto msg) noexcept { m_srcMgr.on_error(tok, msg); });

    m_parser.on_error([this](auto&& err) noexcept { m_srcMgr.on_parse_error(err); });
    m_parser.on_command([this](auto command) noexcept { m_cmd.on_command(std::move(command)); });

    init_commands();
  }


  // Public members

  void driver::run_interactive() noexcept
  {
    tnac::buf_t input;
    m_running = true;

    while(m_running)
    {
      out() << ">> ";
      std::getline(in(), input);
      if (utils::ltrim(input).empty())
      {
        out() << "Enter an expression\n";
        continue;
      }

      parse(std::move(input), true);
      input = {};
    }
  }

  // Protected members

  driver& driver::set_istream(in_stream& stream) noexcept
  {
    m_in = &stream;
    return *this;
  }

  driver& driver::set_ostream(out_stream& stream) noexcept
  {
    m_out = &stream;
    return *this;
  }

  // Friends

  out_stream& operator<<(out_stream& stream, driver& drv) noexcept
  {
    drv.set_ostream(stream);
    return stream;
  }

  in_stream& operator>>(in_stream& stream, driver& drv) noexcept
  {
    drv.set_istream(stream);
    return stream;
  }

  // Private members

  // Command handlers

  void driver::on_exit() noexcept
  {
    m_running = false;
  }

  void driver::print_result() noexcept
  {
    out::value_printer vp;
    vp(m_registry.evaluation_result(), out());
    out() << "\n\n";
  }

  // Utility

  in_stream& driver::in() noexcept
  {
    return *m_in;
  }
  out_stream& driver::out() noexcept
  {
    return *m_out;
  }
  out_stream& driver::err() noexcept
  {
    return *m_err;
  }

  void driver::init_commands() noexcept
  {
    m_commands.declare("exit"sv,   [this](auto) noexcept { on_exit(); });
    m_commands.declare("result"sv, [this](auto) noexcept { print_result(); });
  }

  void driver::parse(tnac::buf_t input, bool interactive) noexcept
  {
    auto&& inputData = m_srcMgr.input(std::move(input));
    auto ast = m_parser(inputData.m_buf);
    inputData.m_node = ast;
    
    eval ev{ m_registry };
    if (!interactive)
      ast = m_parser.root();

    ev(ast);

#if 0
    out::ast_printer pr;
    pr(m_parser.root(), out());
    out() << '\n';

    out::lister ls;
    ls(m_parser.root(), out());
    out() << '\n';
#endif
  }
}