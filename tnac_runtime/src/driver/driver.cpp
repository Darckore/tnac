#include "driver/driver.hpp"
#include "output/printer.hpp"
#include "output/lister.hpp"

namespace tnac_rt
{
  // Special members

  driver::driver(int argCount, char** args) noexcept :
    m_ev{ 1000 }, // todo: configurable
    m_cmd{ m_commands }
  {
    init_handlers();
    init_commands();
    run(argCount, args);
  }


  // Public members


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

  void driver::init_handlers() noexcept
  {
    m_cmd.on_error([this](auto&& tok, auto msg) noexcept { m_srcMgr.on_error(tok, msg); });

    m_ev.on_error([this](auto&& tok, auto msg) noexcept { m_srcMgr.on_error(tok, msg); });

    m_parser.on_variable_declaration([this](auto&& sym) noexcept { store_var(sym); });
    m_parser.on_parse_error([this](auto&& err) noexcept { m_srcMgr.on_parse_error(err); });
    m_parser.on_command([this](auto command) noexcept { m_cmd.on_command(std::move(command)); });
  }

  void driver::run(int argCount, char** args) noexcept
  {
    if (argCount < 2)
    {
      run_interactive();
      return;
    }

    run(args[1]);

    bool interactive{};
    for (auto argIdx = 2; argIdx < argCount; ++argIdx)
    {
      tnac::string_t arg = args[argIdx];
      if (arg == "-i")
        interactive = true;
    }

    if (interactive)
      run_interactive();
  }

  void driver::run(tnac::string_t fileName) noexcept
  {
    fsys::path fn{ fileName };

    std::error_code errc;
    fn = fsys::absolute(fn, errc);
    if (errc)
    {
      err() << "Path '" << fn.string() << "' is invalid. " << errc.message() << '\n';
      return;
    }

    tnac::buf_t buf;
    std::ifstream in{ fn.string() };
    if (!in)
    {
      err() << "Unable to open the input file\n";
      return;
    }

    in.seekg(0, std::ios::end);
    buf.reserve(in.tellg());
    in.seekg(0, std::ios::beg);

    using it = std::istreambuf_iterator<tnac::buf_t::value_type>;
    buf.assign(it{ in }, it{});

    parse(std::move(buf), false);
  }

  void driver::run_interactive() noexcept
  {
    tnac::buf_t input;
    m_running = true;

    while (m_running)
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

  // Command handlers

  void driver::on_exit() noexcept
  {
    m_running = false;
  }

  void driver::set_num_base(int base) noexcept
  {
    m_numBase = base;
  }

  void driver::print_result() noexcept
  {
    out::value_printer vp;
    vp(m_ev.last_result(), m_numBase, out());
    out() << '\n';
  }

  void driver::print_result(command c) noexcept
  {
    static constexpr auto bin = "bin"sv;
    static constexpr auto oct = "oct"sv;
    static constexpr auto dec = "dec"sv;
    static constexpr auto hex = "hex"sv;

    auto base = m_numBase;
    if (c.arg_count())
    {
      auto&& arg     = c[size_type{}];
      auto argName = arg.m_value;
      if (argName == bin)
        base = 2;
      else if (argName == oct)
        base = 8;
      else if (argName == dec)
        base = 10;
      else if (argName == hex)
        base = 16;
      else
        m_srcMgr.on_error(arg, "Unknown parameter"sv);
    }

    tnac::value_guard _{ m_numBase, base };
    print_result();
  }

  void driver::list_code(command c) noexcept
  {
    tnac::value_guard _{ m_out };
    if (c.arg_count())
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
      constexpr auto maxArgs = size_type{ 2 };
      const auto argCount    = c.arg_count();
      
      if (argCount < maxArgs)
        return m_parser.root();

      auto&& second = c[size_type{ 1 }];
      if (second.m_value == "current"sv)
        return m_srcMgr.last_parsed();

      m_srcMgr.on_error(second, "Unknown parameter"sv);
      return m_parser.root();
    };

    auto ast = toPrint(c);
    tnac::value_guard _{ m_out };
    if (c.arg_count())
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
    if (c.arg_count())
    {
      try_redirect_output(c[size_type{}]);
    }

    if (m_vars.empty())
      out() << "<none>\n";

    out::value_printer vp;
    for (auto var : m_vars)
    {
      out() << var->name() << " : ";
      vp(var->value(), 10, out());
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
    m_commands.declare("result"sv, params{ Identifier }, size_type{},
                       [this](auto c) noexcept { print_result(std::move(c)); });
    
    m_commands.declare("list"sv, params{ String }, size_type{},
                       [this](auto c) noexcept { list_code(std::move(c)); });
    
    m_commands.declare("ast"sv, params{ String, Identifier }, size_type{},
                       [this](auto c) noexcept { print_ast(std::move(c)); });

    m_commands.declare("vars"sv, params{ String }, size_type{},
                       [this](auto c) noexcept { print_vars(std::move(c)); });

    m_commands.declare("bin"sv, [this](auto) noexcept { set_num_base(2); });
    m_commands.declare("oct"sv, [this](auto) noexcept { set_num_base(8); });
    m_commands.declare("dec"sv, [this](auto) noexcept { set_num_base(10); });
    m_commands.declare("hex"sv, [this](auto) noexcept { set_num_base(16); });
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
    {
      ast = m_parser.root();
      m_ev(ast);
    }

    if (interactive && ast != m_parser.root())
    {
      m_ev(ast);
    }

    print_result();
  }
}