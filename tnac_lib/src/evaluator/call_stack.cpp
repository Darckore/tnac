#include "evaluator/call_stack.hpp"

namespace tnac::eval
{
  namespace detail
  {
    class stack_frame final
    {
    public:
      using func_sym   = call_stack::sym_t;
      using value_list = call_stack::value_list;
      using size_type  = call_stack::size_type;
      using entity_ref = call_stack::var_t&;
      using entity_id  = call_stack::var_t*;
      using var_values = std::unordered_map<entity_id, temporary>;

    public:
      CLASS_SPECIALS_NODEFAULT_NOCOPY(stack_frame);

      stack_frame(const func_sym& func) noexcept :
        m_func{ &func }
      {}

      value_list& args() noexcept
      {
        return m_args;
      }

      void save_value_for(entity_ref ent, temporary prev) noexcept
      {
        m_prevVars[&ent] = std::move(prev);
      }

      void reapply_vars(value_visitor& vis) noexcept
      {
        for (auto&& [sym, val] : m_prevVars)
        {
          sym->eval_result(vis.visit_assign(sym, *val));
        }
      }

      void allocate(size_type count) noexcept
      {
        m_args.reserve(count);
      }

      auto name() const noexcept
      {
        return m_func->name();
      }

    private:
      const func_sym* m_func{};
      value_list m_args;
      var_values m_prevVars;
    };
  }

  // Special members

  call_stack::~call_stack() noexcept = default;

  call_stack::call_stack(size_type depth) noexcept :
    m_depth{ depth }
  {
    m_frames.reserve(depth);
  }

  call_stack::operator bool() const noexcept
  {
    return can_push();
  }


  // Public members

  void call_stack::store_var(var_t& sym, temporary prev) noexcept
  {
    if (m_frames.empty())
      return;

    auto&& frame = m_frames.back();
    frame.save_value_for(sym, std::move(prev));
  }

  void call_stack::push(const sym_t& callable, const args_t& args, vis_t& visitor) noexcept
  {
    auto&& frame = m_frames.emplace_back(callable);
    frame.allocate(args.size());
    auto&& storedArgs = frame.args();

    // Push previous values of parameters to the stack for later
    // We traverse them in reverse in order to place them in the natural order
    // rather than backwards as usual
    for (auto param : callable.params() | views::reverse)
    {
      auto&& prmSym = param->symbol();
      visitor.push_value(prmSym.value());
    }

    // Push current args on the stack, also in the natural order
    for (auto&& arg : args)
    {
      visitor.push_value(*arg);
      storedArgs.emplace_back(visitor.fetch_next());
    }
  }

  void call_stack::prologue(const sym_t& callable, vis_t& visitor) noexcept
  {
    if (m_frames.empty())
    {
      UTILS_ASSERT(false);
      return;
    }

    auto&& top = m_frames.back();
    for (auto&& [param, arg] : utils::make_iterators(callable.params(), top.args()))
    {
      auto&& sym = param->symbol();
      sym.eval_result(visitor.visit_assign(&sym, *arg));
    }
  }

  void call_stack::epilogue(const sym_t& callable, vis_t& visitor) noexcept
  {
    if (m_frames.empty())
    {
      return;
    }

    for (auto param : callable.params())
    {
      auto&& sym = param->symbol();
      auto prev = visitor.fetch_next();
      sym.eval_result(visitor.visit_assign(&sym, *prev));
    }

    auto&& frame = m_frames.back();
    frame.reapply_vars(visitor);
    m_frames.pop_back();
  }

  void call_stack::clear() noexcept
  {
    m_frames.clear();
  }


  // Private members

  bool call_stack::can_push() const noexcept
  {
    return m_frames.size() < m_depth;
  }

}