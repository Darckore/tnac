#include "driver/driver.hpp"
#include "output/printer.hpp"
#include "output/lister.hpp"

namespace tnac_rt
{
  // Special members

  driver::driver() noexcept :
    m_parser{ m_builder, m_sema },
    m_ev{ m_registry },
    m_cmd{ m_commands }
  {
    m_sema.on_variable([this](auto&& sym) noexcept { store_var(sym); });

    m_cmd.on_error([this](auto&& tok, auto msg) noexcept { m_srcMgr.on_error(tok, msg); });

    m_ev.on_error([this](auto&& tok, auto msg) noexcept { m_srcMgr.on_error(tok, msg); });

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
    out() << '\n';
    out::value_printer vp;
    vp(m_registry.evaluation_result(), out());
    out() << "\n\n";
  }

  void driver::list_code(command c) noexcept
  {
    tnac::value_guard _{ m_out };
    if (c.param_count())
    {
      try_redirect_output(c[size_type{}]);
    }

    out::lister ls;
    ls(m_parser.root(), out());
    end_redirect();
  }

  void driver::print_ast(command c) noexcept
  {
    auto toPrint = [this](const command& c) noexcept -> const tnac::ast::node*
    {
      constexpr auto maxParams = size_type{ 2 };
      const auto paramCount = c.param_count();
      
      if (paramCount < maxParams)
        return m_parser.root();

      auto&& second = c[size_type{ 1 }];
      if (second.m_value == "current"sv)
        return m_srcMgr.last_parsed();

      m_srcMgr.on_error(second, "Unknown parameter"sv);
      return m_parser.root();
    };

    auto ast = toPrint(c);
    tnac::value_guard _{ m_out };
    if (c.param_count())
    {
      try_redirect_output(c[size_type{}]);
    }

    out::ast_printer pr;
    pr(ast, out());
    end_redirect();
  }

  void driver::print_vars(command c) noexcept
  {
    tnac::value_guard _{ m_out };
    if (c.param_count())
    {
      try_redirect_output(c[size_type{}]);
    }

    if (m_vars.empty())
      out() << "<none>";

    out::value_printer vp;
    for (auto var : m_vars)
    {
      out() << var->name() << " : ";
      vp(var->value(), out());
      out() << '\n';
    }
    end_redirect();
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

  void driver::try_redirect_output(const tnac::token& pathTok) noexcept
  {
    if (pathTok.m_value.empty())
      return;

    fsys::path outPath{ pathTok.m_value };
    m_outFile.open(outPath);
    if (!m_outFile)
    {
      m_srcMgr.on_error(pathTok, "Failed to open output file"sv);
      return;
    }

    m_out = &m_outFile;
  }

  void driver::end_redirect() noexcept
  {
    if (m_outFile)
      m_outFile.close();
  }

  void driver::init_commands() noexcept
  {
    using enum tnac::tok_kind;
    using params = tnac::commands::descr::param_list;
    using size_type = params::size_type;

    m_commands.declare("exit"sv,   [this](auto  ) noexcept { on_exit(); });
    m_commands.declare("result"sv, [this](auto  ) noexcept { print_result(); });
    
    m_commands.declare("list"sv, params{ String }, size_type{},
                       [this](auto c) noexcept { list_code(std::move(c)); });
    
    m_commands.declare("ast"sv, params{ String, Identifier }, size_type{},
                       [this](auto c) noexcept { print_ast(std::move(c)); });

    m_commands.declare("vars"sv, params{ String }, size_type{},
                       [this](auto c) noexcept { print_vars(std::move(c)); });
  }

  void driver::store_var(variable_ref var) noexcept
  {
    m_vars.push_back(&var);
  }

  void driver::parse(tnac::buf_t input, bool interactive) noexcept
  {
    auto&& inputData = m_srcMgr.input(std::move(input));
    auto ast = m_parser(inputData.m_buf);
    inputData.m_node = ast;
    
    if (!interactive)
      ast = m_parser.root();

    if(interactive && ast != m_parser.root())
      m_ev(ast);
  }
}