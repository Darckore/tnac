#pragma once
#include "packages/core.hpp"

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
  using arr  = tnac::eval::array_type;
  using func = tnac::eval::function_type;

  inline testing::Message& operator<<(testing::Message& msg, const frac& f) noexcept
  {
    if (f.sign() < 0) msg << '-';
    msg << f.num() << ',' << f.denom();
    return msg;
  }
  inline testing::Message& operator<<(testing::Message& msg, const func& f) noexcept
  {
    msg << "function(" << f->name() << ')';
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

    static auto from_file(string_t fname, bool doEval = false) noexcept
    {
      return value_checker{ fname, doEval };
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

    static void verify(tnac::eval::value val, arr expected) noexcept
    {
      auto cv = val.try_get<tnac::eval::array_type>();
      if (!cv)
      {
        FAIL() << "Checked value is not an array";
        return;
      }

      auto&& checked = *cv;
      if (checked->size() != expected->size())
      {
        FAIL() << "Wrong size. Expected: " << checked->size() << " got: " << expected->size();
        return;
      }

      for (auto&& [c, e] : utils::make_iterators(*checked, *expected))
      {
        auto checkedVal  = *c;
        auto expectedVal = *e;
        tnac::eval::on_value(expectedVal, [checkedVal](auto expVal) noexcept
          {
            verify(checkedVal, std::move(expVal));
          });
      }
    }

    template <testable T>
    static void check(string_t input, T expected) noexcept
    {
      value_checker checker{};
      checker(input, std::move(expected));
    }

    static void check(string_t input) noexcept
    {
      check(input, tnac::eval::invalid_val_t{});
    }

  private:
    static void on_eval_error(const tnac::token& tok, string_t msg) noexcept
    {
      FAIL() << "Eval error " << msg << " at " << tok.m_value;
    }
    static void on_parse_error(const tnac::ast::error_expr& err) noexcept
    {
      FAIL() << "Parse error " << err.message() << " at " << err.pos().m_value;
    }

  private:
    value_checker() noexcept :
      m_tnac{ 128 }
    {
      m_tnac.on_parse_error(on_parse_error);
      m_tnac.on_semantic_error(on_eval_error);
    }

    value_checker(string_t fname, bool doEval = false) noexcept :
      value_checker{}
    {
      read_file(fname);
      if (doEval)
        eval(m_buffer);
    }

  public:
    value_type eval(string_t input) noexcept
    {
      return core().evaluate(input);
    }

    template <testable T>
    void operator()(string_t input, T expected) noexcept
    {
      auto res = eval(input);
      verify(res, std::move(expected));
    }

    template <testable T>
    void operator()(T expected) noexcept
    {
      operator()(m_buffer, std::move(expected));
    }

    void operator()(string_t input) noexcept
    {
      operator()(input, tnac::eval::invalid_val_t{});
    }

    void operator()() noexcept
    {
      operator()(m_buffer);
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

  inline auto read_program(string_t fname, bool doEval = false) noexcept
  {
    return value_checker::from_file(fname, doEval);
  }

  template <testable T>
  inline void verify_program(string_t fname, T expected) noexcept
  {
    auto vc = read_program(fname);
    vc(std::move(expected));
  }

  inline void verify_program(string_t fname) noexcept
  {
    verify_program(fname, tnac::eval::invalid_val_t{});
  }


  //
  // Arrays
  //

  class array_builder final
  {
  public:
    using value_type = tnac::eval::array_type;
    using arr_t      = value_type::value_type;
    using arr_store  = std::forward_list<arr_t>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(array_builder);

    array_builder() noexcept = default;

  public:
    arr_t& add(std::size_t prealloc) noexcept
    {
      auto&& arr = m_arrays.emplace_front();
      arr.reserve(prealloc);
      return arr;
    }

    value_type to_array_type(arr_t& arr) noexcept
    {
      return { arr, ~std::size_t{} };
    }

  private:
    arr_store m_arrays;
  };

}