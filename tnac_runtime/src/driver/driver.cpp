#include "driver/driver.hpp"
#include "output/printer.hpp"
#include "output/lister.hpp"
#include "output/formatting.hpp"

namespace tnac::rt
{
  // Special members

  driver::~driver() noexcept = default;

  driver::driver(int argCount, char** args) noexcept
  {
    set_callbacks();
    m_settings.parse(argCount, args);
    run();
  }


  // Private members

  void driver::run() noexcept
  {
    if (!m_settings.has_input_file())
    {
      // todo: REPL
      return;
    }

    auto loadResult = m_srcMgr.load(m_settings.run_on());
    if (!loadResult)
    {
      return;
    }

    fsys::current_path((*loadResult)->directory());
    // todo: compile

    if (!m_settings.interactive())
      return;

    // todo: REPL
  }


  // Private members (IO)
  
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


  // Private members (Callbacks)

  void driver::set_callbacks() noexcept
  {
    m_settings.on_error([this](string_t msg) noexcept { on_cli_error(msg); });
  }

  void driver::on_cli_error(string_t msg) noexcept
  {
    err() << "<Command line> ";
    fmt::print(err(), fmt::clr::BoldRed, "error: ");
    err() << msg << '\n';
  }

}


#if 0
namespace tnac::rt
{
  // Special members

  driver::driver(int argCount, char** args) noexcept :
    m_tnac{ 2000 }, // todo: configurable
    m_inpMgr{ m_tnac.get_parser().get_src_mgr() }
  {
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
    m_tnac.on_command_error([this](auto&& tok, auto msg) noexcept { m_inpMgr.on_error(tok, msg); });

    m_tnac.on_semantic_error([this](auto&& tok, auto msg) noexcept { m_inpMgr.on_error(tok, msg); });

    m_tnac.on_variable_declaration([this](auto&& sym) noexcept { store_var(sym); });
    m_tnac.on_parse_error([this](auto&& err) noexcept { m_inpMgr.on_parse_error(err); });
    m_tnac.on_command([this](auto command) noexcept { m_tnac.get_commands().on_command(std::move(command)); });
  }

  void driver::run(int argCount, char** args) noexcept
  {
    m_state = {};

    if (argCount < 2)
    {
      run_interactive();
      return;
    }

    for (auto argIdx = 2; argIdx < argCount; ++argIdx)
    {
      tnac::string_t arg = args[argIdx];
      if (arg == "-i")
        m_state.interactive = true;
      else if (arg == "-c")
        m_state.compile = true;
      else if (arg == "-O")
        m_state.optimise = true;
    }

    if (m_state.optimise && !m_state.compile)
    {
      err() << "<Command line> ";
      colours::add_clr(err(), colours::clr::Red, true);
      err() << "error: ";
      colours::clear_clr(err());
      err() << "-O requires -c to be specified\n";
    }

    run(args[1]);
  }

  void driver::run(tnac::string_t fileName) noexcept
  {
    auto input = m_inpMgr.from_file(fileName);
    if (!input)
      return;

    // Initial parsing of the input file (must be non-interactive)
    {
      VALUE_GUARD(m_state.interactive, false);
      parse(*input);
    }

    if (m_state.interactive)
      run_interactive();
  }

  void driver::run_interactive() noexcept
  {
    tnac::buf_t input;
    m_state.start();
    m_state.interactive = true;

    while (m_state)
    {
      out() << ">> ";
      std::getline(in(), input);
      if (utils::ltrim(input).empty())
      {
        out() << "Enter an expression\n";
        continue;
      }

      parse(m_inpMgr.input(std::move(input)));
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
    out() << "\nEvaluation result: ";
    out::value_printer vp;
    colours::add_clr(out(), colours::clr::White, true);
    if(!m_state.compile)
      vp(m_tnac.get_eval().last_result(), m_state.numBase, out());
    else
      vp(m_tnac.get_compiler().last_result(), m_state.numBase, out());
    colours::clear_clr(out());
    out() << "\n\n";
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
      auto argName = arg.value();
      if (argName == bin)
        base = 2;
      else if (argName == oct)
        base = 8;
      else if (argName == dec)
        base = 10;
      else if (argName == hex)
        base = 16;
      else
        m_inpMgr.on_error(arg, "Unknown parameter"sv);
    }

    VALUE_GUARD(m_state.numBase, base);
    print_result();
  }

  void driver::list_code(command c) noexcept
  {
    VALUE_GUARD(m_io.out);
    auto wrapInLines = true;
    if (c.arg_count())
    {
      wrapInLines = false;
      try_redirect_output(c[size_type{}]);
    }

    out::lister ls;
    if (m_io.is_in_cout()) ls.enable_styles();
    if (wrapInLines) out() << '\n';
    ls(m_tnac.get_parser().root(), out());
    if (wrapInLines) out() << '\n';
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
      if (second.value() == "current"sv)
        return m_state.lastParsed;

      m_inpMgr.on_error(second, "Unknown parameter"sv);
      return parser.root();
    };

    auto ast = toPrint(c);
    VALUE_GUARD(m_io.out);
    auto wrapInLines = true;
    if (c.arg_count())
    {
      wrapInLines = false;
      try_redirect_output(c[size_type{}]);
    }

    out::ast_printer pr;
    if (m_io.is_in_cout()) pr.enable_styles();
    if (wrapInLines) out() << '\n';
    pr(ast, out());
    if (wrapInLines) out() << '\n';
    end_redirect();
  }

  void driver::print_vars(command c) noexcept
  {
    VALUE_GUARD(m_io.out);
    auto wrapInLines = true;
    if (c.arg_count())
    {
      wrapInLines = false;
      try_redirect_output(c[size_type{}]);
    }

    if (m_vars.empty())
    {
      if (wrapInLines) out() << '\n';
      out() << "<none>\n";
      if (wrapInLines) out() << '\n';
      return;
    }

    const auto styles = m_io.is_in_cout();
    out::value_printer vp;
    if (wrapInLines) out() << '\n';
    for (auto var : m_vars)
    {
      if (styles) colours::add_clr(out(), colours::clr::Cyan, false);
      out() << var->name();
      if (styles) colours::clear_clr(out());
      print_var_scope(*var, styles);
      out() << " = ";
      if (styles) colours::add_clr(out(), colours::clr::White, true);
      vp(var->value(), 10, out());
      if (styles) colours::clear_clr(out());
      out() << '\n';
    }
    if (wrapInLines) out() << '\n';
    end_redirect();
  }

  void driver::print_var_scope(variable_ref var, bool styles) noexcept
  {
    auto owner = var.owner_scope().m_node;
    if (!owner)
      return;

    auto enclosing = owner->parent();

    using enum tnac::ast::node_kind;
    out() << " in <";
    if (styles) colours::add_clr(out(), colours::clr::Yellow, false);
    if (!enclosing)
      out() << "global";
    else if (auto fd = enclosing->climb<FuncDecl>())
      out() << "function '" << fd->name() << '\'';
    else
      out() << "local";
    if (styles) colours::clear_clr(out());
    out() << '>';
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
    if (pathTok.value().empty())
      return;

    fsys::path outPath{ pathTok.value() };
    m_io.outFile.open(outPath);
    if (!m_io.outFile)
    {
      m_inpMgr.on_error(pathTok, "Failed to open output file"sv);
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

  void driver::process(tnac::ast::node* root) noexcept
  {
    auto&& ev       = m_tnac.get_eval();
    auto&& compiler = m_tnac.get_compiler();

    if (!m_state.compile)
    {
      ev(root);
      return;
    }

    compiler(root);
  }

  void driver::parse(input_t& inputData) noexcept
  {
    auto&& parser = m_tnac.get_parser();
    auto inputFile = inputData.try_get_file();
    auto ast = inputFile ?
      parser(inputData.buffer(), inputFile->make_location()) :
      parser(inputData.buffer());
    
    if (ast && ast->parent())
    {
      m_state.lastParsed = ast;
    }
    
    if (!m_state.interactive)
    {
      process(parser.root());
    }
    else if (ast != parser.root())
    {
      process(ast);
      print_result();
    }
  }
}
#endif