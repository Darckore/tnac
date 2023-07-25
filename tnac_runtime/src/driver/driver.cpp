#include "driver/driver.hpp"
#include "output/printer.hpp"
#include "output/lister.hpp"

namespace tnac_rt
{
  namespace
  {
    void on_terminate() noexcept
    {
      std::cerr << "std::terminate called\n";
    }

    void on_failed_alloc() noexcept
    {
      std::cerr << "memory allocation failed\n";
    }

    struct handler_reg final
    {
      CLASS_SPECIALS_NONE_CUSTOM(handler_reg);

      handler_reg() noexcept
      {
        utils::set_new(on_failed_alloc);
        utils::set_terminate(-1, on_terminate);
      }
    };

    void set_terminate_handlers() noexcept
    {
      static handler_reg _{};
    }
  }

  // Special members

  driver::driver(int argCount, char** args) noexcept :
    m_tnac{ 7000 } // todo: configurable
  {
    set_terminate_handlers();
    init_handlers();
    init_commands();
    run(argCount, args);
  }


  // Public members


  // Protected members

  driver& driver::set_istream(in_stream& stream) noexcept
  {
    m_io.in = &stream;
    return *this;
  }

  driver& driver::set_ostream(out_stream& stream) noexcept
  {
    m_io.out = &stream;
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
    m_tnac.on_command_error([this](auto&& tok, auto msg) noexcept { m_srcMgr.on_error(tok, msg); });

    m_tnac.on_semantic_error([this](auto&& tok, auto msg) noexcept { m_srcMgr.on_error(tok, msg); });

    m_tnac.on_variable_declaration([this](auto&& sym) noexcept { store_var(sym); });
    m_tnac.on_parse_error([this](auto&& err) noexcept { m_srcMgr.on_parse_error(err); });
    m_tnac.on_command([this](auto command) noexcept { m_tnac.get_commands().on_command(std::move(command)); });
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
    m_state.start();

    while (m_state)
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
    m_state.stop();
  }

  void driver::set_num_base(int base) noexcept
  {
    m_state.numBase = base;
  }

  void driver::print_result() noexcept
  {
    out::value_printer vp;
    vp(m_tnac.get_eval().last_result(), m_state.numBase, out());
    out() << '\n';
  }

  void driver::print_result(command c) noexcept
  {
    static constexpr auto bin = "bin"sv;
    static constexpr auto oct = "oct"sv;
    static constexpr auto dec = "dec"sv;
    static constexpr auto hex = "hex"sv;

    auto base = m_state.numBase;
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

    tnac::value_guard _{ m_state.numBase, base };
    print_result();
  }

  void driver::list_code(command c) noexcept
  {
    tnac::value_guard _{ m_io.out };
    if (c.arg_count())
    {
      try_redirect_output(c[size_type{}]);
    }

    out::lister ls;
    ls(m_tnac.get_parser().root(), out());
    end_redirect();
  }

  void driver::print_ast(command c) noexcept
  {
    auto toPrint = [this](const command& c) noexcept -> const tnac::ast::node*
    {
      constexpr auto maxArgs = size_type{ 2 };
      const auto argCount    = c.arg_count();
      auto&& parser = m_tnac.get_parser();

      if (argCount < maxArgs)
        return parser.root();

      auto&& second = c[size_type{ 1 }];
      if (second.m_value == "current"sv)
        return m_srcMgr.last_parsed();

      m_srcMgr.on_error(second, "Unknown parameter"sv);
      return parser.root();
    };

    auto ast = toPrint(c);
    tnac::value_guard _{ m_io.out };
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
    tnac::value_guard _{ m_io.out };
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
    return *m_io.in;
  }
  out_stream& driver::out() noexcept
  {
    return *m_io.out;
  }
  out_stream& driver::err() noexcept
  {
    return *m_io.err;
  }

  void driver::try_redirect_output(const tnac::token& pathTok) noexcept
  {
    if (pathTok.m_value.empty())
      return;

    fsys::path outPath{ pathTok.m_value };
    m_io.outFile.open(outPath);
    if (!m_io.outFile)
    {
      m_srcMgr.on_error(pathTok, "Failed to open output file"sv);
      return;
    }

    m_io.out = &m_io.outFile;
  }

  void driver::end_redirect() noexcept
  {
    if (m_io.outFile)
      m_io.outFile.close();
  }

  void driver::init_commands() noexcept
  {
    using enum tnac::tok_kind;
    using params = tnac::commands::descr::param_list;
    using size_type = params::size_type;
    auto&& cmd = m_tnac.get_commands();

    cmd.declare("exit"sv,   [this](auto  ) noexcept { on_exit(); });
    cmd.declare("result"sv, params{ Identifier }, size_type{},
                [this](auto c) noexcept { print_result(std::move(c)); });
    
    cmd.declare("list"sv, params{ String }, size_type{},
                [this](auto c) noexcept { list_code(std::move(c)); });
    
    cmd.declare("ast"sv, params{ String, Identifier }, size_type{},
                [this](auto c) noexcept { print_ast(std::move(c)); });

    cmd.declare("vars"sv, params{ String }, size_type{},
                [this](auto c) noexcept { print_vars(std::move(c)); });

    cmd.declare("bin"sv, [this](auto) noexcept { set_num_base(2); });
    cmd.declare("oct"sv, [this](auto) noexcept { set_num_base(8); });
    cmd.declare("dec"sv, [this](auto) noexcept { set_num_base(10); });
    cmd.declare("hex"sv, [this](auto) noexcept { set_num_base(16); });
  }

  void driver::store_var(variable_ref var) noexcept
  {
    m_vars.push_back(&var);
  }

  void driver::parse(tnac::buf_t input, bool interactive) noexcept
  {
    auto&& inputData = m_srcMgr.input(std::move(input));
    auto&& parser = m_tnac.get_parser();
    auto&& ev = m_tnac.get_eval();
    auto ast = parser(inputData.m_buf);
    inputData.m_node = ast;
    
    if (!interactive)
    {
      ast = parser.root();
      ev(ast);
    }

    if (interactive && ast != parser.root())
    {
      ev(ast);
    }

    print_result();
  }
}