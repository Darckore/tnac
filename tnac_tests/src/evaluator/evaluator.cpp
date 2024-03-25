#if 0
//
// Helper object for instantiations
//
template <eval::expr_result T>
class instance
{
public:
  using err_handler_t = evaluator::err_handler_t;
  using arg_list_t = evaluator::arg_list_t;
  using size_type = arg_list_t::size_type;
  using visitor = eval::value_visitor;
  using value_type = T;
  using type_info = eval::type_info<value_type>;

  static constexpr auto min = type_info::minArgs;
  static constexpr auto max = type_info::maxArgs;

public:
  CLASS_SPECIALS_NONE(instance);

  instance(visitor& valVisitor, err_handler_t& onError) noexcept :
    m_visitor{ valVisitor },
    m_errHandler{ onError }
  {}

  void operator()(const ast::typed_expr& expr) noexcept
  {
    if (!check_args(expr))
    {
      for (auto _ : expr.args())
      {
        utils::unused(_);
        m_visitor.fetch_next();
      }
      m_visitor.push_value({});
      return;
    }

    instantiate(expr, std::make_index_sequence<max>{});
  }

private:
  template <typename T, T... Seq>
  void instantiate(const ast::typed_expr& expr, std::integer_sequence<T, Seq...>) noexcept
  {
    auto&& exprArgs = expr.args();
    std::array<eval::stored_value, max> args{};
    m_visitor.fill_args(args, exprArgs.size());
    m_visitor.instantiate<value_type>(std::move(args[Seq])...);
  }

  void on_error(const token& pos, string_t msg) noexcept
  {
    if (m_errHandler)
      m_errHandler(pos, msg);
  }

  bool check_args(const ast::typed_expr& expr) noexcept
  {
    const auto size = expr.args().size();
    if (utils::in_range(size, min, max))
      return true;

    auto msg = std::format("Expected at least {} and at most {} arguments"sv, min, max);
    on_error(expr.type_name(), msg);

    return false;
  }

private:
  visitor& m_visitor;
  err_handler_t& m_errHandler;
};
#endif

#if 0
  void evaluator::visit(ast::typed_expr& expr) noexcept
  {
    if (return_path())
      return;

    using enum tok_kind;
    using detail::instance;

    switch (expr.type_name().what())
    {
    case KwComplex:  instance<eval::complex_type>{ m_visitor, m_errHandler }(expr);  break;
    case KwFraction: instance<eval::fraction_type>{ m_visitor, m_errHandler }(expr); break;
    case KwInt:      instance<eval::int_type>{ m_visitor, m_errHandler }(expr); break;
    case KwFloat:    instance<eval::float_type>{ m_visitor, m_errHandler }(expr); break;
    case KwBool:     instance<eval::bool_type>{ m_visitor, m_errHandler }(expr); break;
    
    default: UTILS_ASSERT(false); break;
    }
  }

  void evaluator::visit(ast::call_expr& expr) noexcept
  {
    if (return_path())
      return;

    auto args = m_visitor.collect_args_locally(expr.args().size());
    auto callee = m_visitor.fetch_next();
    if (auto arr = (*callee).try_get<eval::array_type>())
    {
      make_arr_call(*arr, args, expr);
    }
    else
    {
      auto funcType = (*callee).try_get<eval::function_type>();
      make_call(funcType, args, expr);
    }
  }

  void evaluator::visit(ast::array_expr& arr) noexcept
  {
    if (return_path())
      return;

    const auto arrSz = arr.elements().size();
    m_visitor.make_array(arrSz);
  }

  bool evaluator::preview(ast::cond_expr& expr) noexcept
  {
    if (return_path())
      return false;

    auto&& cond = expr.cond();
    traverse(&cond);
    auto condVal = m_visitor.fetch_next();

    ast::pattern* trueBranch{};
    ast::pattern* defaultBranch{};
    using eval::val_ops;
    for (auto child : expr.patterns().children())
    {
      if (!child->is(ast::node_kind::Pattern))
      {
        m_visitor.clear_result();
        return false;
      }

      auto&& pattern = utils::cast<ast::pattern>(*child);
      
      if (!pattern.matcher().is(ast::node_kind::Matcher))
      {
        m_visitor.clear_result();
        return false;
      }

      auto&& matcher = utils::cast<ast::matcher>(pattern.matcher());
      
      eval::stored_value currentMatch{};
      if (matcher.is_default())
      {
        defaultBranch = &pattern;
        continue;
      }
      else if (matcher.is_unary())
      {
        const auto opcode = detail::conv_unary(matcher.pos().what());
        m_visitor.visit_unary(*condVal, opcode);
        currentMatch = m_visitor.fetch_next();
      }
      else
      {
        auto&& checkedExpr = matcher.checked();
        traverse(&checkedExpr);
        auto checkedVal = m_visitor.fetch_next();
        auto opcode = matcher.has_implicit_op() ?
          val_ops::Equal :
          detail::conv_binary(matcher.pos().what());

        m_visitor.visit_binary(*condVal, *checkedVal, opcode);
        currentMatch = m_visitor.fetch_next();
      }

      if (to_bool(*currentMatch))
      {
        trueBranch = &pattern;
        break;
      }
    }

    if (auto winner = (trueBranch ? trueBranch : defaultBranch))
    {
      if (auto&& body = winner->body(); !body.children().empty())
      {
        traverse(&winner->body());
        m_visitor.push_last();
      }
      else
        m_visitor.clear_result();
    }
    else
    {
      m_visitor.clear_result();
    }

    return false;
  }

  bool evaluator::preview(ast::scope& scope) noexcept
  {
    const auto returning = return_path();
    auto callable = try_get_callable(scope);
    if (!callable)
      return !returning;

    if(!returning)
      m_callStack.prologue(*callable, m_visitor);

    return !returning;
  }

  void evaluator::visit(ast::scope& scope) noexcept
  {
    auto callable = try_get_callable(scope);
    if (!callable)
      return;

    m_visitor.push_last();
    auto resVal = m_visitor.fetch_next();
    m_callStack.epilogue(*callable, m_visitor);
    m_visitor.push_value(*resVal);
  }

  void evaluator::make_arr_call(eval::array_type arr, const arr_t& args, ast::call_expr& expr) noexcept
  {
    auto _ = m_visitor.lock(arr);
    auto resCount = size_type{};
    for (const auto argCount = expr.args().size(); auto&& elem : *arr)
    {
      auto elemValue = *elem;

      if (auto arrCallable = elemValue.try_get<eval::array_type>())
      {
        ++resCount;
        make_arr_call(*arrCallable, args, expr);
        continue;
      }

      auto argFunc = elemValue.try_get<eval::function_type>();
      if (!argFunc || (*argFunc)->param_count() != argCount)
        continue;

      ++resCount;
      make_call(argFunc, args, expr);
    }

    m_visitor.make_array(resCount);
  }

  void evaluator::make_call(eval::function_type* func, const arr_t& args, ast::call_expr& expr) noexcept
  {
    auto&& at = expr.pos();
    if (!func)
    {
      on_error(at, "Expected a callable object"sv);
      return;
    }

    auto callable = *func;
    if (const auto paramCnt = callable->param_count(); paramCnt != expr.args().size())
    {
      on_error(at, std::format("Expected {} arguments"sv, paramCnt));
      return;
    }

    if (!m_callStack)
    {
      on_error(at, "Stack overflow"sv);
      m_visitor.clear_result();
      m_visitor.fetch_next();
      m_fatal = true;
      return;
    }

    m_callStack.push(*callable, args, m_visitor);
    auto funcBody = callable->declarator().definition();
    VALUE_GUARD(m_return);
    traverse(funcBody);
  }

#endif