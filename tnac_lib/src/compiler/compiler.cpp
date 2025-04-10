#include "compiler/compiler.hpp"
#include "common/feedback.hpp"
#include "common/diag.hpp"
#include "sema/sema.hpp"
#include "cfg/cfg.hpp"
#include "eval/value_store.hpp"
#include "eval/type_impl.hpp"

namespace tnac::eval::detail
{
  constexpr auto conv_unary(tok_kind tk) noexcept
  {
    using enum tok_kind;
    using enum eval::val_ops;
    switch (tk)
    {
    case Exclamation: return LogicalNot;
    case Question:    return LogicalIs;
    case Plus:        return UnaryPlus;
    case Minus:       return UnaryNegation;
    case Tilde:       return UnaryBitwiseNot;
    case At:          return UnaryHead;

    default: return InvalidOp;
    }
  }
  constexpr auto conv_binary(tok_kind tk) noexcept
  {
    using enum tok_kind;
    using enum eval::val_ops;
    switch (tk)
    {
    case Plus:     return Addition;
    case Minus:    return Subtraction;
    case Asterisk: return Multiplication;
    case Slash:    return Division;
    case Percent:  return Modulo;

    case Less:      return RelLess;
    case LessEq:    return RelLessEq;
    case Greater:   return RelGr;
    case GreaterEq: return RelGrEq;
    case Eq:        return Equal;
    case NotEq:     return NEqual;

    case Amp:  return BitwiseAnd;
    case Hat:  return BitwiseXor;
    case Pipe: return BitwiseOr;

    case Pow:  return BinaryPow;
    case Root: return BinaryRoot;

    default: return InvalidOp;
    }
  }
}

namespace tnac::detail
{
  constexpr auto to_binary_opcode(tok_kind tk) noexcept
  {
    using enum tok_kind;
    switch (tk)
    {
    case Plus:      return ir::op_code::Add;
    case Minus:     return ir::op_code::Sub;
    case Asterisk:  return ir::op_code::Mul;
    case Slash:     return ir::op_code::Div;
    case Percent:   return ir::op_code::Mod;
    case Amp:       return ir::op_code::And;
    case Pipe:      return ir::op_code::Or;
    case Hat:       return ir::op_code::Xor;
    case Pow:       return ir::op_code::Pow;
    case Root:      return ir::op_code::Root;
    case Less:      return ir::op_code::CmpL;
    case LessEq:    return ir::op_code::CmpLE;
    case Greater:   return ir::op_code::CmpG;
    case GreaterEq: return ir::op_code::CmpGE;
    case Eq:        return ir::op_code::CmpE;
    case NotEq:     return ir::op_code::CmpNE;

    default: break;
    }

    return ir::op_code::None;
  }
  constexpr auto to_unary_opcode(tok_kind tk) noexcept
  {
    using enum tok_kind;
    switch (tk)
    {
    case Exclamation: return ir::op_code::CmpNot;
    case Question:    return ir::op_code::CmpIs;
    case Plus:        return ir::op_code::Plus;
    case At:          return ir::op_code::Head;
    case Minus:       return ir::op_code::Neg;
    case Tilde:       return ir::op_code::BNeg;

    default: break;
    }

    return ir::op_code::None;
  }
  constexpr auto is_land(tok_kind tk) noexcept
  {
    return tk == tok_kind::LogAnd;
  }
  constexpr auto is_lor(tok_kind tk) noexcept
  {
    return tk == tok_kind::LogOr;
  }
  constexpr auto is_logical(tok_kind tk) noexcept
  {
    return is_land(tk) || is_lor(tk);
  }
  constexpr auto logical_to_str(tok_kind tk) noexcept
  {
    switch (tk)
    {
    case tok_kind::LogAnd: return "land"sv;
    case tok_kind::LogOr:  return "lor"sv;
    default: break;
    }
    return string_t{};
  }
  constexpr auto is_assign(tok_kind tk) noexcept
  {
    return utils::eq_any(tk, tok_kind::Assign);
  }

  auto try_rhs(ast::node& node) noexcept -> ast::expr*
  {
    if (auto assign = utils::try_cast<ast::assign_expr>(&node))
      return &assign->right();

    if (auto var = utils::try_cast<ast::var_decl>(&node))
      return &var->initialiser();

    return nullptr;
  }

  template <typename T>
  constexpr auto expected_args() -> std::pair<std::size_t, std::size_t>
  {
    using ti = eval::type_info<T>;
    return { ti::minArgs, ti::maxArgs };
  }

  auto expected_args(const token& tk) noexcept -> std::pair<std::size_t, std::size_t>
  {
    using detail::expected_args;
    using enum tok_kind;

    switch (tk.what())
    {
    case KwComplex:  return expected_args<eval::complex_type>();
    case KwFraction: return expected_args<eval::fraction_type>();
    case KwInt:      return expected_args<eval::int_type>();
    case KwFloat:    return expected_args<eval::float_type>();
    case KwBool:     return expected_args<eval::bool_type>();

    default: UTILS_ASSERT(false); break;
    }
    return {};
  }
  auto to_type_id(const token& tk) noexcept
  {
    using enum tok_kind;

    switch (tk.what())
    {
    case KwComplex:  return eval::type_id::Complex;
    case KwFraction: return eval::type_id::Fraction;
    case KwInt:      return eval::type_id::Int;
    case KwFloat:    return eval::type_id::Float;
    case KwBool:     return eval::type_id::Bool;
    case KwArray:    return eval::type_id::Array;
    case KwFunction: return eval::type_id::Function;
    case KwUndef:    return eval::type_id::Invalid;

    default: UTILS_ASSERT(false); break;
    }
    return eval::type_id::Invalid;
  }

  constexpr auto to_inst_code(eval::type_id ti) noexcept
  {
    using enum eval::type_id;
    switch (ti)
    {
    case Bool:     return ir::op_code::Bool;
    case Int:      return ir::op_code::Int;
    case Float:    return ir::op_code::Float;
    case Fraction: return ir::op_code::Frac;
    case Complex:  return ir::op_code::Cplx;

    default: UTILS_ASSERT(false); break;
    }

    return ir::op_code::None;
  }

  constexpr auto needs_named_reg(ir::op_code oc) noexcept
  {
    using enum ir::op_code;
    return utils::eq_none(oc, Load, Phi, Bool, Int, Float, Frac, Cplx);
  }

  constexpr auto needs_forced_bool(ir::op_code oc) noexcept
  {
    using enum ir::op_code;
    return utils::eq_none(oc, CmpE,   CmpL,  CmpLE, CmpNE, CmpG, CmpGE,
                              CmpNot, CmpIs, Bool,  Test);
  }

  auto needs_forced_bool(const ir::operand& op) noexcept
  {
    if (op.is_value() && op.get_value().id() == eval::type_id::Bool)
      return false;

    if (!op.is_register())
      return true;

    auto&& reg = op.get_reg();
    if (!reg.has_src())
      return true;

    auto&& src = reg.source();
    const auto oc = src.opcode();
    if (oc == ir::op_code::Phi)
    {
      const auto ops = src.operand_count();
      using st = decltype(src.operand_count());
      for (auto count = st{ 1 }; count < ops; ++count)
      {
        auto&& operand = src[count];
        if (!operand.is_edge())
          continue;

        auto&& edge = operand.get_edge();
        if (!needs_forced_bool(edge.value()))
          return false;
      }
    }

    return needs_forced_bool(oc);
  }

  template <typename F>
  concept fv_callback = std::is_nothrow_invocable_v<F, ast::func_decl&>;

  template <fv_callback Func>
  class func_visitor final : public ast::bottom_up_visitor<func_visitor<Func>>
  {
  public:
    CLASS_SPECIALS_NONE(func_visitor);

    func_visitor(Func callback) noexcept :
      m_callback{ std::move(callback) }
    {}

    bool preview(ast::func_decl& fd) noexcept
    {
      if(!fd.name().starts_with('`'))
        m_callback(fd);
      return false;
    }

  private:
    Func m_callback;
  };
}

namespace tnac
{
  // Special members

  compiler::~compiler() noexcept = default;

  compiler::compiler(sema& sema, eval::store& valStore, ir::cfg& gr, feedback* fb) noexcept :
    m_sema{ &sema },
    m_feedback{ fb },
    m_cfg{ &gr },
    m_vals{ &valStore }
  {}


  // Public members

  void compiler::operator()(tree_ref node) noexcept
  {
    if (!node.is_valid())
    {
      error(diag::compilation_stopped());
      return;
    }

    compile(node);
  }

  const ir::cfg& compiler::cfg() const noexcept
  {
    return *m_cfg;
  }
  ir::cfg& compiler::cfg() noexcept
  {
    return FROM_CONST(cfg);
  }


  // Exprs

  void compiler::visit(ast::error_expr& err) noexcept
  {
    // Should never even be here since we break at the first module with errors
    UTILS_ASSERT(false);
    error(err.pos().at(), err.message());
  }

  void compiler::visit(ast::result_expr&) noexcept
  {
    if (auto last = m_context.last_store())
      emit_load(*last);
    if (m_stack.empty())
      m_stack.push_undef();
    else
      m_stack.push(m_stack.top());
  }

  void compiler::visit(ast::ret_expr&) noexcept
  {
    auto retBlock = m_context.return_block();

    if (!retBlock)
    {
      if (m_context.is_in_func_scope())
        return;

      retBlock = &m_context.create_block(m_names.ret_block_name());
      auto&& rv = emit_alloc(m_names.ret_var_name());
      m_context.set_return(*retBlock, rv);
    }

    auto rv = m_context.ret_val();
    if (!check_post_jmp())
    {
      auto op = extract();
      intern_array(op);
      emit_store(*rv, op);
    }
    emit_jump(m_context.ret_val(), *retBlock);
  }

  void compiler::visit(ast::lit_expr& lit) noexcept
  {
    using eval::value;
    auto&& litValue = lit.pos();
    value val{};
    clear_store();
    switch (litValue.what())
    {
    case token::KwTrue:  val = value::true_val();                      break;
    case token::KwFalse: val = value::false_val();                     break;
    case token::KwI:     val = value::i();                             break;
    case token::KwPi:    val = value::pi();                            break;
    case token::KwE:     val = value::e();                             break;
    case token::IntDec:  val = value::parse_int(litValue.value(), 10); break;
    case token::IntBin:  val = value::parse_int(litValue.value(), 2);  break;
    case token::IntOct:  val = value::parse_int(litValue.value(), 8);  break;
    case token::IntHex:  val = value::parse_int(litValue.value(), 16); break;
    case token::Float:   val = value::parse_float(litValue.value());   break;
    case token::KwUndef: val = value{};                                break;

    default: m_stack.push_undef(); return;
    }

    m_stack.push(val);
  }

  void compiler::visit(ast::id_expr& id) noexcept
  {
    auto sym = &id.symbol();
    if (auto ref = utils::try_cast<semantics::scope_ref>(sym))
    {
      sym = ref->referenced().to_callable();
    }

    if(!sym)
      return;

    if (auto func = m_cfg->find_entity(sym))
    {
      m_stack.push(eval::value::function(*func));
      return;
    }

    UTILS_ASSERT(!sym->is(semantics::sym_kind::Deferred));
    emit_load(*sym);
  }

  void compiler::visit(ast::unary_expr& unary) noexcept
  {
    auto val = extract();
    const auto opType = unary.op().what();
    compile_unary(val, opType);
  }

  void compiler::visit(ast::tail_expr& ) noexcept
  {
    auto val = extract();
    compile_unary(val, eval::val_ops::PostTail, ir::op_code::Tail);
  }

  void compiler::visit(ast::type_check_expr& unary) noexcept
  {
    auto val = extract();
    compile_test(val, detail::to_type_id(unary.type()));
  }

  void compiler::visit(ast::binary_expr& binary) noexcept
  {
    const auto opType = binary.op().what();
    if (detail::is_logical(opType) || detail::is_assign(opType))
      return;

    auto rhs = extract();
    auto lhs = extract();
    compile_binary(lhs, rhs, opType);
  }

  void compiler::visit(ast::array_expr& arr) noexcept
  {
    const auto size = arr.elements().size();
    if (!m_stack.has_values(size))
    {
      clear_store();
      auto&& target = emit_arr(size);
      emit_append(target, size);
      emit_load(target);
      return;
    }

    auto&& arrData = m_vals->allocate_array(size);
    m_stack.fill(arrData, size);
    auto&& arrWrapper = m_vals->wrap(arrData);
    m_stack.push(eval::value::array(arrWrapper));
  }

  void compiler::visit(ast::abs_expr&) noexcept
  {
    auto val = extract();
    if (val.is_value())
    {
      clear_store();
      auto&& sv = val.get_value();
      m_stack.push(sv.unary(eval::val_ops::AbsoluteValue));
      return;
    }

    emit_unary(ir::op_code::Abs, val);
  }

  void compiler::visit(ast::typed_expr& typed) noexcept
  {
    auto&& args = typed.args();
    const auto argSz = args.size();
    const auto argLimits = detail::expected_args(typed.type_name());
    if (!utils::in_range(argSz, argLimits.first, argLimits.second))
    {
      const auto threshold = argSz < argLimits.first ? argLimits.first : argLimits.second;
      error(typed.pos().at(), diag::wrong_arg_num(threshold, argSz));
      m_stack.drop(argSz);
      m_stack.push_undef();
      return;
    }

    const auto typeId = detail::to_type_id(typed.type_name());
    if (m_stack.has_values(argSz))
    {
      m_stack.instantiate(typeId, argSz);
      return;
    }

    emit_inst(detail::to_inst_code(typeId), argLimits.second, argSz);
  }

  bool compiler::exit_child(ast::node& node) noexcept
  {
    return !(node.is(ast::node_kind::Ret) || has_ret_jump());
  }

  void compiler::post_exit(ast::node& node) noexcept
  {
    auto loc = try_get_location(node);
    if (!loc) return;
    warning(*loc, diag::unreachable());
  }

  // Decls

  void compiler::visit(ast::param_decl& param) noexcept
  {
    auto&& sym = param.symbol();
    emit_alloc(sym);
    emit_store(sym);
  }

  void compiler::visit(ast::func_decl& fd) noexcept
  {
    auto&& sym = fd.symbol();
    auto func = m_cfg->find_entity(&sym);
    UTILS_ASSERT(func);
    clear_store();
    m_stack.push(eval::value::function(*func));
  }

  // Previews

  bool compiler::preview(ast::root& root) noexcept
  {
    auto&& modules = root.modules();
    auto ok = true;
    for (auto mod : modules)
    {
      if (!mod->is_valid())
      {
        error(diag::compilation_stopped(mod->name()));
        ok = false;
        break;
      }

      auto&& modSym = mod->symbol();
      m_context.store(modSym, *mod);
      m_context.push(modSym);
      if (ok = walk_imports(*mod); !ok)
        break;
    }

    if (ok)
      compile_modules();

    m_context.wipe();
    return false;
  }

  bool compiler::preview(ast::func_decl& fd) noexcept
  {
    auto lastVal = m_stack.try_extract();
    auto&& owner = m_context.current_function();
    const auto parCnt = fd.param_count();
    auto funcName = m_names.mangle_func_name(fd.name(), owner, parCnt);
    auto&& func = m_cfg->declare_function(&fd.symbol(), owner, funcName, parCnt);
    m_context.enter_function(func, fd.body());
    compile(fd.params(), fd.body().children());
    m_context.exit_function();
    if(!lastVal.is_undef())
      m_stack.push(lastVal);
    return false;
  }

  bool compiler::preview(ast::var_decl& var) noexcept
  {
    auto&& sym = var.symbol();
    emit_alloc(sym);
    compile_init(sym, var.initialiser());
    return false;
  }

  bool compiler::preview(ast::assign_expr& assign) noexcept
  {
    auto target = utils::try_cast<ast::id_expr>(&assign.left());
    UTILS_ASSERT(target);
    compile_init(target->symbol(), assign.right());
    return false;
  }

  bool compiler::preview(ast::binary_expr& binary) noexcept
  {
    const auto opType = binary.op().what();
    if (!detail::is_logical(opType))
      return true;

    auto alwaysSame = [&](const eval::value& val, bool isLhs) noexcept
      {
        const auto boolVal = eval::to_bool(val);
        const auto knownVal = ( boolVal && detail::is_lor(opType)) || // always true
                              (!boolVal && detail::is_land(opType));  // always false

        if(knownVal)
        {
          auto binOp = binary.op();
          warning(binOp.at(), diag::logical_same(binOp.value(), isLhs, boolVal));
          m_stack.push(eval::value{ boolVal });
          return true;
        }
        return false;
      };

    compile(binary.left());
    enforce_bool(extract());
    auto leftOp = extract();
    if (leftOp.is_value())
    {
      if (!alwaysSame(leftOp.get_value(), true))
      {
        compile(binary.right());
      }
      return false;
    }

    auto&& lastBlock = m_context.current_block();
    auto lastEnd = m_context.func_end();

    auto opName = detail::logical_to_str(opType);
    auto&& rhsBlock = m_context.create_block(m_names.make_block_name(opName, "rhs"sv));
    m_context.enter_block(rhsBlock);
    m_context.terminate_at(rhsBlock);
    compile(binary.right());

    enforce_bool(extract());
    auto rightOp = extract();
    if (rightOp.is_value() && !has_ret_jump(rhsBlock))
    {
      m_context.enter_block(lastBlock);
      m_context.override_last(lastBlock.end());
      m_context.terminate_at(lastBlock);
      if (!alwaysSame(rightOp.get_value(), false))
      {
        m_stack.push(leftOp);
      }

      m_context.override_last(lastEnd);
      m_context.current_function().delete_block_tree(rhsBlock);
      return false;
    }

    auto&& endBlock = m_context.create_block(m_names.make_block_name(opName, "end"sv));
    lastEnd = m_context.override_last(lastBlock.end());
    m_context.enter_block(lastBlock);
    if(detail::is_lor(opType))
      emit_cond_jump(leftOp, endBlock, rhsBlock);
    else
      emit_cond_jump(leftOp, rhsBlock, endBlock, eval::value::false_val());

    auto finalBlock = m_context.terminal_block();
    UTILS_ASSERT(finalBlock);
    lastEnd = m_context.override_last(finalBlock->end());
    m_context.enter_block(*finalBlock);
    emit_jump(rightOp, endBlock);

    m_context.enter_block(endBlock);
    m_context.terminate_at(endBlock);
    converge();

    return false;
  }

  bool compiler::preview(ast::type_resolve_expr& tres) noexcept
  {
    auto&& checker = tres.checker();
    compile(checker.operand());
    auto onFalse = extract();
    compile_test(onFalse, detail::to_type_id(checker.type()));
    auto checkedVal = extract();
    if (checkedVal.is_value())
    {
      auto&& sv = checkedVal.get_value();
      auto boolVal = eval::to_bool(sv);
      if (!boolVal)
        m_stack.push(onFalse);
      else
        compile(tres.resolver());

      return false;
    }

    constexpr auto namePref = "ontype"sv;
    auto&& lastBlock = m_context.current_block();
    auto lastEnd = m_context.func_end();

    auto&& endBlock = m_context.create_block(m_names.make_block_name(namePref, "end"sv));

    auto&& passBlock = m_context.create_block(m_names.make_block_name(namePref, "pass"sv));
    m_context.enter_block(passBlock);
    m_context.terminate_at(passBlock);
    compile(tres.resolver());
    auto trueRes = extract();
    emit_jump(trueRes, endBlock);

    auto&& elseBlock = m_context.create_block(m_names.make_block_name(namePref, "else"sv));
    m_context.enter_block(elseBlock);
    m_context.terminate_at(elseBlock);
    m_stack.push(onFalse);
    emit_jump(onFalse, endBlock);

    lastEnd = m_context.override_last(lastBlock.end());
    m_context.enter_block(lastBlock);

    emit_cond_jump(checkedVal, passBlock, elseBlock);

    m_context.enter_block(endBlock);
    m_context.terminate_at(endBlock);
    converge();

    return false;
  }

  bool compiler::preview(ast::cond_short& cond) noexcept
  {
    compile(cond.cond());
    enforce_bool(extract());
    auto checkedVal = extract();
    if (checkedVal.is_value())
    {
      auto&& sv = checkedVal.get_value();
      auto boolVal = eval::to_bool(sv);
      warning(cond.cond().pos().at(), diag::condition_same(boolVal));
      if (boolVal && !cond.has_true())
      {
        m_stack.push(checkedVal);
      }
      else if (!boolVal && !cond.has_false())
      {
        m_stack.push_undef();
      }
      else
      {
        auto&& condPart = boolVal ? cond.on_true() : cond.on_false();
        compile(condPart);
      }

      return false;
    }

    constexpr auto namePref = "shortcnd"sv;
    auto&& lastBlock = m_context.current_block();
    auto lastEnd = m_context.func_end();

    auto&& endBlock = m_context.create_block(m_names.make_block_name(namePref, "end"sv));

    m_stack.push(checkedVal);
    auto&& onTrue  = m_context.create_block(m_names.make_block_name(namePref, "true"sv));
    m_context.enter_block(onTrue);
    m_context.terminate_at(onTrue);
    if (!cond.has_true())
      m_stack.push(checkedVal);
    else
      compile(cond.on_true());
    auto trueRes = extract();
    extract();
    emit_jump(trueRes, endBlock);

    m_stack.push(checkedVal);
    auto&& onFalse = m_context.create_block(m_names.make_block_name(namePref, "false"sv));
    m_context.enter_block(onFalse);
    m_context.terminate_at(onFalse);
    if (!cond.has_false())
      m_stack.push_undef();
    else
      compile(cond.on_false());
    auto falseRes = extract();
    extract();
    emit_jump(falseRes, endBlock);

    lastEnd = m_context.override_last(lastBlock.end());
    m_context.enter_block(lastBlock);
    bool knownBranchValues = trueRes.is_value() && falseRes.is_value();
    knownBranchValues = knownBranchValues && !has_ret_jump(onTrue) && !has_ret_jump(onFalse);
    if (knownBranchValues)
    {
      emit_select(checkedVal, trueRes, falseRes);
      auto&& curFn = m_context.current_function();
      curFn.delete_block_tree(onTrue);
      curFn.delete_block_tree(onFalse);
      m_context.terminate_at(lastBlock);
      m_context.override_last(lastEnd);
      return false;
    }

    emit_cond_jump(checkedVal, onTrue, onFalse);

    m_context.enter_block(endBlock);
    m_context.terminate_at(endBlock);
    converge();

    return false;
  }

  bool compiler::preview(ast::cond_expr& cond) noexcept
  {
    compile(cond.cond());
    auto checkedVal = extract();
    constexpr auto namePref = "cond"sv;
    auto&& endBlock = m_context.create_block(m_names.make_block_name(namePref, "exit"sv));
    auto&& patterns = cond.patterns().children();
    ast::pattern* defaultPat{};
    for (auto counter = patterns.size(); auto child : patterns)
    {
      auto&& pattern = utils::cast<ast::pattern>(*child);
      if (auto&& matcher = utils::cast<ast::matcher>(pattern.matcher()); matcher.is_default())
      {
        defaultPat = &pattern;
        continue;
      }
      --counter;

      m_context.terminate_at(endBlock);
      if (!compile(pattern, checkedVal, !counter))
        continue;

      if (endBlock.preds().empty() && delete_block_tree(endBlock))
      {
        m_context.terminate_at(m_context.current_block());
      }
      else
      {
        emit_jump(extract(), endBlock);
        m_context.enter_block(endBlock);
        converge();
      }
      return false;
    }

    if (defaultPat)
    {
      m_context.terminate_at(endBlock);
      if(endBlock.preds().empty() && delete_block_tree(endBlock))
      {
        m_context.terminate_at(m_context.current_block());
        compile(defaultPat->body());
        return false;
      }

      compile(*defaultPat, checkedVal, true);
    }

    converge();
    return false;
  }

  bool compiler::preview(ast::call_expr& call) noexcept
  {
    auto&& args = call.args();
    const auto argSz = args.size();
    compile(call.callable());
    auto callable = extract();
    if (callable.is_value())
    {
      auto callVal = callable.get_value();
      if (auto arr = callVal.try_get<eval::array_type>())
      {
        for (auto arg : args)
          compile(*arg);

        emit_arr_call(call, *arr);
        return false;
      }

      auto fn = callVal.try_get<eval::function_type>();
      if (!fn)
      {
        error(call.pos().at(), diag::expected("function"sv));
        m_stack.push_undef();
        return false;
      }

      auto&& sym = *(*fn);
      const auto expSz = sym.param_count();
      if (argSz != expSz)
      {
        error(call.pos().at(), diag::wrong_arg_num(expSz, argSz));
        m_stack.push_undef();
        return false;
      }
    }

    for (auto arg : args)
      compile(*arg);

    emit_call(std::move(callable), argSz);
    return false;
  }

  bool compiler::preview(ast::dot_expr& dot) noexcept
  {
    UTILS_ASSERT(dot.accessor().is(ast::node_kind::Identifier));
    auto&& accr = utils::cast<ast::id_expr>(dot.accessor());
    if (auto&& sym = accr.symbol(); !sym.is(semantics::sym_kind::Deferred))
    {
      compile(accr);
      return false;
    }

    compile(dot.accessed());
    auto scope = m_stack.extract();
    emit_dyn(std::move(scope), accr.name());
    return false;
  }

  // Private members (Emitions)

  void compiler::update_func_start(ir::instruction& instr) noexcept
  {
    m_context.func_start_at(instr);
  }

  void compiler::clear_store() noexcept
  {
    m_context.clear_store();
  }

  void compiler::converge() noexcept
  {
    auto preds = m_context.current_block().preds();
    if (preds.empty())
      return;

    // Corner case, some degenerate branch here
    if (preds.size() == 1)
    {
      m_stack.push(preds.front()->value());
      return;
    }

    emit_phi(preds);
  }

  ir::instruction& compiler::make(ir::op_code oc, size_opt prealloc /*= {}*/) noexcept
  {
    clear_store();
    using enum ir::op_code;

    auto&& builder = m_cfg->get_builder();
    auto&& block = m_context.current_block();
    auto&& instr = prealloc ?
      builder.add_instruction(block, oc, *prealloc, m_context.func_end()):
      builder.add_instruction(block, oc, m_context.func_end());

    auto&& res = detail::needs_named_reg(oc) ?
      builder.make_register(m_names.op_name(oc)):
      builder.make_register(m_context.register_index());

    instr.add(&res);
    update_func_start(instr);
    m_stack.push(&res);
    return instr;
  }

  void compiler::emit_alloc(semantics::symbol& sym) noexcept
  {
    auto varName = sym.name();
    auto&& reg = emit_alloc(varName);
    m_context.store(sym, reg);
  }

  ir::vreg& compiler::emit_alloc(string_t varName) noexcept
  {
    auto&& curFn = m_context.current_function();
    auto&& entry = curFn.entry();
    auto&& builder = m_cfg->get_builder();
    auto&& var = builder.add_var(entry, m_context.funct_start());
    if (!m_context.new_var_name(varName))
    {
      varName = m_names.var_name(varName);
    }

    auto&& reg = builder.make_register(varName);
    var.add(&reg);
    return reg;
  }

  ir::vreg& compiler::emit_arr(ir::operand::idx_type size) noexcept
  {
    auto&& curFn = m_context.current_function();
    auto&& entry = curFn.entry();
    auto&& builder = m_cfg->get_builder();
    auto&& arr = builder.add_array(entry, m_context.funct_start());
    auto varName = m_names.op_name(ir::op_code::Arr);
    auto&& reg = builder.make_register(varName);
    arr.add(&reg);
    arr.add(size);
    return reg;
  }

  void compiler::emit_ret(ir::basic_block& block) noexcept
  {
    auto op = m_stack.extract();
    intern_array(op);
    auto&& instr = m_cfg->get_builder().add_instruction(block, ir::op_code::Ret, m_context.func_end());
    instr.add(std::move(op));
    update_func_start(instr);
    m_context.exit_block();
  }

  void compiler::emit_store(semantics::symbol& var) noexcept
  {
    auto target = m_context.locate(var);
    UTILS_ASSERT(target);

    // Most likely, unreachable code
    // like trying to get result from a condition whose
    // branches all explicitly return (including default)
    if (m_stack.empty())
      return;
    
    auto val = m_stack.extract();
    intern_array(val);
    if(!val.is_param())
      m_context.save_store(var);

    emit_store(*target, val);
    m_context.modify(var);
  }

  void compiler::emit_store(ir::vreg& target, ir::operand val) noexcept
  {
    auto&& block = m_context.current_block();
    auto&& instr = m_cfg->get_builder().add_instruction(block, ir::op_code::Store, m_context.func_end());
    instr.add(val).add(&target);
    update_func_start(instr);
  }

  void compiler::emit_load(semantics::symbol& var) noexcept
  {
    if (auto res = m_context.last_read(var))
    {
      clear_store();
      m_stack.push(res);
      return;
    }

    auto target = m_context.locate(var);
    UTILS_ASSERT(target);

    auto&& res = emit_load(*target);
    m_context.read_into(var, res);
  }

  ir::vreg& compiler::emit_load(ir::vreg& target) noexcept
  {
    auto&& instr = make(ir::op_code::Load).add(&target);
    auto&& res = instr[0];
    return res.get_reg();
  }

  void compiler::emit_binary(ir::op_code oc, ir::operand lhs, ir::operand rhs) noexcept
  {
    make(oc).add(std::move(lhs)).add(std::move(rhs));
  }

  void compiler::emit_unary(ir::op_code oc, ir::operand val) noexcept
  {
    make(oc).add(std::move(val));
  }

  void compiler::emit_test(ir::operand val, eval::type_id id) noexcept
  {
    make(ir::op_code::Test).add(id).add(std::move(val));
  }

  void compiler::emit_jump(ir::operand value, ir::basic_block& dest) noexcept
  {
    if (has_ret_jump())
    {
      return;
    }

    clear_store();
    auto&& block = m_context.current_block();
    auto&& instr = m_cfg->get_builder().add_instruction(block, ir::op_code::Jump, m_context.func_end());
    instr.add(&dest);
    intern_array(value);
    m_cfg->connect(block, dest, value);
    update_func_start(instr);
  }

  void compiler::emit_cond_jump(ir::operand cond, ir::basic_block& ifTrue, ir::basic_block& ifFalse, eval::value falseV /*= {}*/) noexcept
  {
    clear_store();
    auto&& block = m_context.current_block();
    auto&& instr = m_cfg->get_builder().add_instruction(block, ir::op_code::Jump, m_context.func_end());
    instr.add(cond).add(&ifTrue).add(&ifFalse);
    m_cfg->connect(block, ifTrue, cond);
    m_cfg->connect(block, ifFalse, falseV);
    update_func_start(instr);
  }

  void compiler::emit_phi(edge_view edges) noexcept
  {
    auto&& instr = make(ir::op_code::Phi, edges.size() + 1);
    for (auto edge : edges)
    {
      instr.add(edge);
    }
  }

  void compiler::emit_select(ir::operand cond, ir::operand onTrue, ir::operand onFalse) noexcept
  {
    make(ir::op_code::Select).add(cond).add(onTrue).add(onFalse);
  }

  void compiler::emit_inst(ir::op_code oc, size_type opCount, size_type factCount) noexcept
  {
    UTILS_ASSERT(factCount <= opCount);
    auto&& instr = make(oc);
    auto res = extract();
    m_stack.fill(instr, factCount);
    while (factCount < opCount)
    {
      instr.add(eval::value{ eval::int_type{} });
      ++factCount;
    }
    m_stack.push(res);
  }

  void compiler::emit_append(ir::vreg& arr, size_type size) noexcept
  {
    clear_store();
    UTILS_ASSERT(m_stack.has_at_least(size));
    auto&& block = m_context.current_block();
    auto&& builder = m_cfg->get_builder();
    auto before = m_context.func_end();

    for (auto cur = size; cur; --cur)
    {
      auto&& instr = builder.add_instruction(block, ir::op_code::Append, before);
      auto val = extract();
      intern_array(val);
      instr.add(val).add(&arr);
      update_func_start(instr);
      before = instr.to_iterator();
    }
  }

  void compiler::emit_arr_call(ast::call_expr& call, eval::array_type arr) noexcept
  {
    auto&& args = call.args();
    const auto argSz = args.size();
    using vals_t = std::vector<ir::operand>;
    vals_t argVals;
    argVals.reserve(argSz);

    auto pushBack = [&]() noexcept
      {
        for (auto arg : argVals)
          m_stack.push(std::move(arg));
      };

    m_stack.fill(argVals, argSz);
    vals_t::size_type count{};
    for (auto&& callee : arr.wrapper())
    {
      if (auto arrT = callee.try_get<eval::array_type>())
      {
        pushBack();
        emit_arr_call(call, *arrT);

        auto res = m_stack.extract();
        if (res.is_value() && !res.get_value())
          continue;

        m_stack.push(std::move(res));
        ++count;
        continue;
      }

      auto ft = callee.try_get<eval::function_type>();
      if (!ft)
        continue;

      auto&& func = *ft;
      if (func->param_count() != argSz)
        continue;

      pushBack();
      emit_call(callee, argSz);
      ++count;
    }

    if (!count)
    {
      m_stack.push_undef();
      return;
    }

    auto&& aReg = emit_arr(count);
    emit_append(aReg, count);
    emit_load(aReg);
  }

  void compiler::emit_call(ir::operand callable, size_type argCount) noexcept
  {
    const auto size = argCount;
    UTILS_ASSERT(m_stack.has_at_least(size));

    auto&& instr = make(ir::op_code::Call, size + 2); // result + callable + args
    auto res = extract();
    instr.add(std::move(callable));
    m_stack.fill(instr, size);
    intern_array(instr);
    m_stack.push(std::move(res));
  }

  void compiler::emit_dyn(ir::operand scope, string_t name) noexcept
  {
    make(ir::op_code::DynBind).add(std::move(scope)).add(name);
  }

  // Private members

  bool compiler::has_ret_jump() noexcept
  {
    return has_ret_jump(m_context.current_block());
  }

  bool compiler::has_ret_jump(ir::basic_block& block) noexcept
  {
    auto retBlock = m_context.return_block();
    if (!retBlock)
      return false;

    return block.is_connected_to(*retBlock);
  }

  bool compiler::check_post_jmp() noexcept
  {
    auto&& block = m_context.current_block();
    auto lastInstr = block.last();
    using enum ir::op_code;
    return lastInstr && utils::eq_any(lastInstr->opcode(), Jump, Ret);
  }

  bool compiler::delete_block_tree(ir::basic_block& root) noexcept
  {
    auto retBlock = m_context.return_block();
    if (retBlock && root.is_connected_to(*retBlock))
      return false;

    m_context.current_function().delete_block_tree(root);
    return true;
  }

  ir::operand compiler::extract() noexcept
  {
    if (!m_stack.empty())
      return m_stack.extract();

    if (auto last = m_context.last_store())
    {
      emit_load(*last);
      return m_stack.extract();
    }

    return eval::value{};
  }

  void compiler::empty_stack() noexcept
  {
    while (!m_stack.empty())
    {
      // todo: warning - dead code
      m_stack.pop();
    }
  }

  void compiler::enforce_bool(const ir::operand& op) noexcept
  {
    if (detail::needs_forced_bool(op))
    {
      compile_unary(op, tok_kind::Question);
      return;
    }

    m_stack.push(op);
  }

  void compiler::compile_init(semantics::symbol& sym, ast::expr& init) noexcept
  {
    const auto stackSz = m_stack.size();
    compile(init);
    transfer_last_load(stackSz);
    if (!check_post_jmp())
      emit_store(sym);
  }

  void compiler::compile_unary(const ir::operand& val, tok_kind opType) noexcept
  {
    const auto op = eval::detail::conv_unary(opType);
    const auto opcode = detail::to_unary_opcode(opType);
    compile_unary(val, op, opcode);
  }

  void compiler::compile_test(const ir::operand& op, eval::type_id id) noexcept
  {
    // A hack for expressions that are guaranteed to return bool
    const auto isBool = id == eval::type_id::Bool;
    const auto boolExpr = !detail::needs_forced_bool(op);
    if (boolExpr && isBool)
    {
      m_stack.push(eval::value::true_val());
      return;
    }
    else if (boolExpr && !isBool)
    {
      m_stack.push(eval::value::false_val());
      return;
    }

    if (op.is_value())
    {
      auto opVal = op.get_value();
      m_stack.push(eval::value{ opVal.id() == id });
      return;
    }

    emit_test(op, id);
  }

  void compiler::compile_unary(const ir::operand& val, eval::val_ops opType, ir::op_code oc) noexcept
  {
    if (val.is_value())
    {
      clear_store();
      auto&& sv = val.get_value();
      m_stack.push(sv.unary(opType));
      return;
    }

    UTILS_ASSERT(oc != ir::op_code::None);
    emit_unary(oc, val);
  }

  void compiler::compile_binary(const ir::operand& lhs, const ir::operand& rhs, tok_kind opType) noexcept
  {
    if (lhs.is_value() && rhs.is_value())
    {
      clear_store();
      const auto op = eval::detail::conv_binary(opType);
      auto&& lv = lhs.get_value();
      auto&& rv = rhs.get_value();
      m_stack.push(lv.binary(op, rv));
      return;
    }

    const auto opcode = detail::to_binary_opcode(opType);
    UTILS_ASSERT(opcode != ir::op_code::None);
    emit_binary(opcode, lhs, rhs);
  }

  bool compiler::compile(ast::pattern& pattern, const ir::operand& checked, bool last) noexcept
  {
    auto&& matcher = utils::cast<ast::matcher>(pattern.matcher());
    auto term = m_context.terminal_block();
    UTILS_ASSERT(term);
    const auto op = matcher.has_implicit_op() ? tok_kind::Eq : matcher.pos().what();
    const auto isDefault = matcher.is_default();
    if (matcher.is_unary())
    {
      compile_unary(checked, op);
    }
    else if(!isDefault)
    {
      compile(matcher.checked());
      auto rhs = extract();
      compile_binary(checked, rhs, op);
    }
    else
    {
      compile(pattern.body());
      emit_jump(extract(), *term);
      m_context.enter_block(*term);
      m_context.terminate_at(*term);
      return true;
    }

    auto checkRes = extract();
    if (isDefault || checkRes.is_value())
    {
      auto&& sv = checkRes.get_value();
      const auto matchFound = isDefault || eval::to_bool(sv);
      if (!matchFound)
        return false;

      compile(pattern.body());
      return true;
    }

    constexpr auto namePref = "cond"sv;
    auto&& condThen = m_context.create_block(m_names.make_block_name(namePref, "then"sv));
    auto&& condElse = last ?
      *term :
      m_context.create_block(m_names.make_block_name(namePref, "else"sv));

    emit_cond_jump(checkRes, condThen, condElse);
    m_context.enter_block(condThen);
    compile(pattern.body());
    emit_jump(extract(), *term);
    m_context.enter_block(condElse);
    m_context.terminate_at(*term);
    return false;
  }

  void compiler::compile(params_t& params, body_t& body) noexcept
  {
    auto _ = m_names.init_indicies();
    auto&& entry = m_context.create_block(m_names.entry_block_name());
    m_context.enter_block(entry);
    for (ir::function::size_type idx{}; auto param : params)
    {
      m_stack.push(ir::func_param{ idx++ });
      compile(*param);
    }

    compile(body);

    if (m_stack.empty())
    {
      m_stack.push_undef(); // Undefined, probably, an empty body
    }

    auto block = &m_context.terminal_or_entry();
    if (auto last = m_context.last_store())
      emit_load(*last);

    if (auto retBlock = m_context.return_block())
    {
      auto rv = m_context.ret_val();
      if (!has_ret_jump())
      {
        auto op = extract();
        intern_array(op);
        emit_store(*rv, op);
        emit_jump(rv, *retBlock);
      }

      block = retBlock;
      m_context.enter_block(*block);
      m_context.terminate_at(*block);
      auto&& loadRes = emit_load(*rv);
      m_stack.push(&loadRes);
    }

    emit_ret(*block);
  }

  void compiler::compile(ast::scope& scope) noexcept
  {
    auto&& lastScope = m_context.enter_scope(scope);
    compile(scope.children());
    m_context.enter_scope(lastScope);
  }

  void compiler::compile(body_t& body) noexcept
  {
    if (body.empty())
    {
      m_stack.push_undef();
      return;
    }

    bool compileExprs = true;
    bool reportExit = false;
    ast::node* ret{};
    for (auto child : body)
    {
      bool hasFunc{};
      if (compileExprs)
      {
        compile(*child);
      }
      else
      {
        hasFunc = compile_funcs(*child);
      }

      if (!reportExit && !exit_child(*child))
      {
        if (auto rhs = detail::try_rhs(*child))
          child = rhs;

        ret = child;
        reportExit = true;
        compileExprs = false;
        continue;
      }

      if (compileExprs)
        continue;

      hasFunc = hasFunc || compile_funcs(*child);
      if (!hasFunc && reportExit)
      {
        reportExit = false;
        post_exit(*child);
        auto loc = try_get_location(*ret);
        if (!loc)
          continue;

        if (ret->is(ast::node_kind::Ret))
          note(std::move(*loc), diag::ret_here());
        else if (ret->is(ast::node_kind::Cond))
          note(std::move(*loc), diag::all_branches_return());
      }
    }
  }

  bool compiler::compile_funcs(ast::node& expr) noexcept
  {
    bool res = false;
    detail::func_visitor fv{ [&](ast::func_decl& fd) noexcept
      {
        preview(fd);
        res = true;
      } };

    fv(&expr);
    return res;
  }

  void compiler::compile(semantics::module_sym& mod) noexcept
  {
    // This module has already been compiled
    if (m_cfg->find_entity(&mod))
      return;

    auto def = m_context.locate(mod);
    UTILS_ASSERT(def);

    const auto parCnt = mod.param_count();
    auto modName = m_names.mangle_module_name(mod, parCnt);
    auto&& irMod = m_cfg->declare_module(&mod, modName, parCnt);

    register_import_scopes(irMod, *def);

    m_context.enter_function(irMod, *def);
    compile(def->params(), def->children());
    m_context.exit_function();
  }

  void compiler::compile(tree_ref node) noexcept
  {
    base_t::operator()(&node);
  }

  void compiler::compile_modules() noexcept
  {
    while (auto mod = m_context.pop())
    {
      compile(*mod);
      empty_stack();
    }
  }

  semantics::module_sym* compiler::get_module(ast::import_dir& imp) noexcept
  {
    auto sym = &imp.imported_sym();
    auto modSym = utils::try_cast<semantics::module_sym>(sym);
    if (!modSym)
    {
      if (auto ref = utils::try_cast<semantics::scope_ref>(sym))
        modSym = ref->referenced().to_module();
    }

    return modSym;
  }

  bool compiler::walk_imports(ast::module_def& mod) noexcept
  {
    for (auto imp : mod.imports())
    {
      auto modSym = get_module(*imp);
      if (!modSym)
      {
        error(imp->name().back()->pos().at(), diag::undef_id());
        return false;
      }

      m_context.push(*modSym);
    }

    return true;
  }

  void compiler::register_import_scopes(ir::function& mod, ast::module_def& def) noexcept
  {
    auto&& builder = m_cfg->get_builder();
    for (auto imp : def.imports())
    {
      auto curFn = &mod;
      auto&& importedSym = imp->imported_sym();
      for (auto part : imp->name())
      {
        auto&& sym = part->symbol();
        if (&sym != &importedSym)
        {
          if (auto existing = curFn->lookup(sym.name()))
          {
            curFn = existing;
          }
          else
          {
            auto&& loose = builder.make_loose(&sym, sym.name());
            curFn->add_child_name(loose);
            curFn = &loose;
          }
          continue;
        }

        if (curFn == &mod)
          continue;

        if(auto importedMod = m_cfg->find_entity(&importedSym))
          curFn->add_child_name(importedSym.name(), *importedMod);
      }

      auto alias = imp->alias_name();
      if (!alias)
        continue;

      auto alSym = utils::try_cast<semantics::scope_ref>(&alias->symbol());
      UTILS_ASSERT(alSym);
      if (auto aliasFn = m_cfg->find_entity(alSym->referenced().to_module()))
        mod.add_child_name(alias->name(), *aliasFn);
    }
  }

  void compiler::transfer_last_load(size_type prevSz) noexcept
  {
    if (prevSz != m_stack.size())
      return;

    if (auto last = m_context.last_store())
    {
      emit_load(*last);
    }
  }

  void compiler::intern_array(const ir::operand& op) noexcept
  {
    if (!op.is_value())
      return;

    intern_array(op.get_value());
  }

  void compiler::intern_array(const ir::instruction& instr) noexcept
  {
    using sz = ir::instruction::size_type;
    for (auto count = sz{}; count < instr.operand_count(); ++count)
      intern_array(instr[count]);
  }

  void compiler::intern_array(eval::value val) noexcept
  {
    auto arrPtr = val.try_get<eval::array_type>();
    if (!arrPtr)
      return;

    auto arr = *arrPtr;
    for (auto it = arr->begin(); it != arr->end(); ++it)
    {
      intern_array(*it);
    }

    auto&& builder = m_cfg->get_builder();
    auto&& reg = builder.make_global_register(m_names.array_name());
    builder.intern(reg, std::move(arr));
  }

  void compiler::error(string_t msg) noexcept
  {
    if (!m_feedback)
      return;

    m_feedback->error(msg);
  }

  void compiler::error(src::loc_wrapper loc, string_t msg) noexcept
  {
    if (!m_feedback)
      return;

    m_feedback->compile_error(std::move(loc), msg);
  }

  void compiler::warning(src::loc_wrapper loc, string_t msg) noexcept
  {
    if (!m_feedback)
      return;

    m_feedback->compile_warning(std::move(loc), msg);
  }

  void compiler::note(src::loc_wrapper loc, string_t msg) noexcept
  {
    if (!m_feedback)
      return;

    m_feedback->compile_note(std::move(loc), msg);
  }
}