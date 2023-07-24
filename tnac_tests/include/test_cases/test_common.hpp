#pragma once
#include "packages/core.hpp"

namespace tnac_tests
{
  namespace detail
  {
    namespace tree = tnac::ast;
    using tree::node_kind;
    using tnac::string_t;

    struct parse_helper
    {
      parse_helper() :
        parser{ builder, sema }
      {}

      auto operator()(string_t input) noexcept
      {
        return parser(input);
      }

      tnac::ast::builder builder;
      tnac::sema sema;
      tnac::parser parser;
    };

    using cplx = tnac::eval::complex_type;
    using frac = tnac::eval::fraction_type;

    template <tnac::eval::detail::expr_result T>
    inline bool eq(const T& l, const T& r) noexcept
    {
      return l == r;
    }

    template <>
    inline bool eq(const tnac::eval::float_type& l, const tnac::eval::float_type& r) noexcept
    {
      if (std::isinf(l) && std::isinf(r))
        return true;

      if (std::isnan(l) && std::isnan(r))
        return true;

      return utils::eq(l, r);
    }

    template <>
    inline bool eq(const cplx& l, const cplx& r) noexcept
    {
      return eq(l.real(), r.real()) && eq(l.imag(), r.imag());
    }

    inline testing::Message& operator<<(testing::Message& msg, const frac& f) noexcept
    {
      if (f.sign() < 0) msg << '-';
      msg << f.num() << ',' << f.denom();
      return msg;
    }

    inline auto parse_input(string_t input, tnac::eval::registry& reg, tnac::eval::call_stack& cs) noexcept
    {
      parse_helper p;
      p.parser(input);
      tnac::evaluator ev{ reg, cs };
      ev(p.parser.root());

      return reg.evaluation_result();
    }

    inline constexpr auto invalid_value() noexcept
    {
      return tnac::eval::invalid_val_t{};
    }

    template <typename T>
    concept testable_val = tnac::eval::detail::expr_result<T> ||
                           tnac::is_same_noquals_v<T, tnac::eval::invalid_val_t>;

    template <testable_val T>
    inline void verify(tnac::eval::value val, T expected) noexcept
    {
      if constexpr (tnac::is_same_noquals_v<T, tnac::eval::invalid_val_t>)
      {
        ASSERT_TRUE(!val);
      }
      else
      {
        tnac::eval::on_value(val, [expected](auto val) noexcept
          {
            using expected_t = decltype(val);
            if constexpr (tnac::is_same_noquals_v<expected_t, tnac::eval::invalid_val_t>)
            {
              ASSERT_TRUE(false) << "Undefined value detected";
            }
            else if constexpr (!tnac::is_same_noquals_v<expected_t, T>)
            {
              ASSERT_TRUE(false) << "Wrong value type";
            }
            else
            {
              ASSERT_TRUE(eq(expected, val)) << "expected: " << expected << " got: " << val;
            }
          });
      }
    }

    template <tnac::eval::detail::expr_result T>
    inline void check_eval(string_t input, T expected) noexcept
    {
      tnac::eval::registry reg;
      tnac::eval::call_stack cs{ 10 };
      verify(parse_input(input, reg, cs), expected);
    }

    inline void check_invalid(string_t input) noexcept
    {
      tnac::eval::registry reg;
      tnac::eval::call_stack cs{ 0 };
      verify(parse_input(input, reg, cs), invalid_value());
    }

    inline auto to_bool(tnac::eval::value val) noexcept -> std::optional<bool>
    {
      if (!val) return {};
      return tnac::eval::cast_value<bool>(val);
    }
  }
}

namespace tnac_tests
{
  //
  // Stuff
  //

  namespace tree = tnac::ast;
  namespace pkg  = tnac::packages;
  using tree::node_kind;
  using tnac::string_t;
  using tnac::buf_t;
  using tnac::tok_kind;
  using tnac::ast::node_kind;
  using cplx = tnac::eval::complex_type;
  using frac = tnac::eval::fraction_type;

  inline testing::Message& operator<<(testing::Message& msg, const frac& f) noexcept
  {
    if (f.sign() < 0) msg << '-';
    msg << f.num() << ',' << f.denom();
    return msg;
  }

  inline pkg::tnac_core get_tnac(std::size_t stackSz = 0) noexcept
  {
    return pkg::tnac_core{ stackSz };
  }


  //
  // Lexer
  //

  inline void all_same(string_t input, tok_kind kind)
  {
    tnac::lex lex;
    lex(input);

    for (;;)
    {
      auto tok = lex.next();
      if (tok.is_eol())
        break;

      EXPECT_TRUE(tok.is(kind)) << "Failed token: " << tok.m_value;
    }
  }
  
  template <std::size_t N>
  void check_tokens(string_t input, const std::array<tok_kind, N>& tokArr) noexcept
  {
    tnac::lex lex;
    lex(input);

    for (auto tk : tokArr)
    {
      auto tok = lex.next();
      EXPECT_TRUE(tok.is(tk)) << "Failed token: " << tok.m_value;
    }
  }


  //
  // Parser
  //

  class tree_checker : public tree::const_bottom_up_visitor<tree_checker>
  {
  public:
    struct expected_node
    {
      string_t data{};
      node_kind kind{};
      node_kind parent{};
      bool nullParent{};
    };

    using test_data = std::span<expected_node>;
    using data_iter = test_data::iterator;

    static void check_tree_structute(std::span<expected_node> exp, string_t input) noexcept
    {
      auto core = get_tnac();
      auto ast = core.get_parser()(input);
      tree_checker{ exp }(ast);
    }

    template <node_kind kind, std::size_t N>
    static void check_simple_exprs(const std::array<string_t, N>& inputs)
    {
      auto core = get_tnac();
      for (auto input : inputs)
      {
        auto ast = core.get_parser()(input);
        EXPECT_NE(ast, nullptr) << "Null AST for input: " << input;

        if (ast)
        {
          const auto nodeKind = ast->what();
          EXPECT_EQ(nodeKind, kind) << "Bad kind for input: " << input;
          if (nodeKind == kind)
          {
            auto&& tok = static_cast<tree::expr&>(*ast).pos();
            EXPECT_TRUE(input.starts_with(tok.m_value));
          }
        }
      }
    }

    static void check_error(tnac::string_t input, tnac::string_t errMsg) noexcept
    {
      auto core = get_tnac();
      core.on_parse_error(on_error);
      expectedErr = errMsg;
      stop = false;

      core.get_parser()(input);

      expectedErr = {};
      stop = false;
    }

  private:
    inline static string_t expectedErr{};
    inline static bool stop{};

    static void on_error(const tree::error_expr& err) noexcept
    {
      if (stop) return;

      if (err.message() == expectedErr)
      {
        stop = true;
        return;
      }

      if (err.message() != expectedErr)
      {
        ASSERT_EQ(err.message(), expectedErr);
      }
    }

  public:
    CLASS_SPECIALS_NONE(tree_checker);

    tree_checker(test_data expected) noexcept :
      m_data{ expected },
      m_iter{ expected.begin() }
    {}

  public:
    void visit(const tree::scope& scope) noexcept
    {
      check_node(scope, "");
    }

    void visit(const tree::assign_expr& expr) noexcept
    {
      check_node(expr, expr.op().m_value);
    }

    void visit(const tree::decl_expr& expr) noexcept
    {
      check_node(expr, "");
    }

    void visit(const tree::var_decl& decl) noexcept
    {
      check_node(decl, decl.name());
    }

    void visit(const tree::param_decl& decl) noexcept
    {
      check_node(decl, decl.name());
    }

    void visit(const tree::func_decl& decl) noexcept
    {
      check_node(decl, decl.name());
    }

    void visit(const tree::binary_expr& expr) noexcept
    {
      check_node(expr, expr.op().m_value);
    }

    void visit(const tree::unary_expr& expr) noexcept
    {
      check_node(expr, expr.op().m_value);
    }

    void visit(const tree::paren_expr& expr) noexcept
    {
      check_node(expr, "");
    }

    void visit(const tree::abs_expr& expr) noexcept
    {
      check_node(expr, "");
    }

    void visit(const tree::typed_expr& expr) noexcept
    {
      check_node(expr, expr.type_name().m_value);
    }

    void visit(const tree::call_expr& expr) noexcept
    {
      check_node(expr, "");
    }

    void visit(const tree::lit_expr& expr) noexcept
    {
      check_node(expr, expr.pos().m_value);
    }

    void visit(const tree::id_expr& expr) noexcept
    {
      check_node(expr, expr.name());
    }

    void visit(const tree::ret_expr& expr) noexcept
    {
      check_node(expr, "");
    }

    void visit(const tree::result_expr& expr) noexcept
    {
      check_node(expr, expr.pos().m_value);
    }

    void visit(const tree::error_expr& expr) noexcept
    {
      check_node(expr, expr.message());
    }

  private:
    void check_node(const tree::node& node, string_t nodeStr) noexcept
    {
      ASSERT_NE(m_iter, m_data.end()) << "Unexpected end of data";
      auto&& expected = *m_iter;
      ++m_iter;

      EXPECT_EQ(nodeStr, expected.data);

      const auto expKind = node.what();
      EXPECT_EQ(expKind, expected.kind) << "Wrong kind at node " << nodeStr;

      auto parent = node.parent();
      if (!parent)
      {
        EXPECT_TRUE(expected.nullParent) << "Parent was null at node " << nodeStr;
      }
      else
      {
        EXPECT_EQ(parent->what(), expected.parent) << "Wrong parent at node " << nodeStr;
      }
    }

  private:
    test_data m_data{};
    data_iter m_iter{};
  };

  using expected_node = tree_checker::expected_node;


  //
  // Evaluator
  //

  template <typename T>
  concept testable = tnac::eval::detail::generic_type<T>;

  class value_checker final
  {
  public:
    using value_type = tnac::eval::value;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(value_checker);

    static constexpr auto infinity() noexcept
    {
      return std::numeric_limits<tnac::eval::float_type>::infinity();
    }
    static constexpr auto nan() noexcept
    {
      return std::numeric_limits<tnac::eval::float_type>::quiet_NaN();
    }

    static auto from_file(string_t fname) noexcept
    {
      return value_checker{ fname };
    }

    template <testable T>
    static void verify(tnac::eval::value val, T expected) noexcept
    {
      if constexpr (tnac::is_same_noquals_v<T, tnac::eval::invalid_val_t>)
      {
        ASSERT_TRUE(!val);
      }
      else
      {
        tnac::eval::on_value(val, [expected](auto val) noexcept
          {
            using expected_t = decltype(val);
            if constexpr (tnac::is_same_noquals_v<expected_t, tnac::eval::invalid_val_t>)
            {
              ASSERT_TRUE(false) << "Undefined value detected";
            }
            else if constexpr (!tnac::is_same_noquals_v<expected_t, T>)
            {
              ASSERT_TRUE(false) << "Wrong value type";
            }
            else
            {
              using tnac::eval::eq;
              ASSERT_TRUE(eq(expected, val)) << "expected: " << expected << " got: " << val;
            }
          });
      }
    }

    template <testable T>
    static void check(string_t input, T expected) noexcept
    {
      value_checker checker{};
      auto res = checker(input);
      verify(res, std::move(expected));
    }

    static void check(string_t input) noexcept
    {
      check(input, tnac::eval::invalid_val_t{});
    }

  private:
    value_checker() noexcept :
      m_tnac{ 128 }
    {}

    value_checker(string_t fname) noexcept :
      value_checker{}
    {
      read_file(fname);
    }

  public:
    value_type operator()(string_t input) noexcept
    {
      return core().evaluate(input);
    }

  private:
    void read_file(string_t fname) noexcept
    {
      fsys::path fn{ fname };

      std::error_code errc;
      fn = fsys::absolute(fn, errc);
      ASSERT_FALSE(static_cast<bool>(errc)) << "Bad file name " << fname;
      std::ifstream in{ fn.string() };
      ASSERT_TRUE(static_cast<bool>(in));
      in.seekg(0, std::ios::end);
      m_buffer.reserve(in.tellg());
      in.seekg(0, std::ios::beg);
      using it = std::istreambuf_iterator<tnac::buf_t::value_type>;
      m_buffer.assign(it{ in }, it{});
    }

    pkg::tnac_core& core() noexcept
    {
      return m_tnac;
    }

  private:
    pkg::tnac_core m_tnac;
    buf_t m_buffer;
  };
}