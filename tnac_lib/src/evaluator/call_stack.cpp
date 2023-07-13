#include "evaluator/call_stack.hpp"

namespace tnac::eval
{
  namespace detail
  {
    class stack_frame final
    {
    public:
      using func_name  = call_stack::func_name;
      using value_list = call_stack::value_list;
      using size_type  = call_stack::size_type;

    public:
      CLASS_SPECIALS_NODEFAULT(stack_frame);

      stack_frame(func_name name) noexcept :
        m_name{ name }
      {}

      value_list& args() noexcept
      {
        return m_args;
      }

      value_list& params() noexcept
      {
        return m_params;
      }

      void allocate(size_type count) noexcept
      {
        m_params.reserve(count);
        m_args.reserve(count);
      }

      func_name name() const noexcept
      {
        return m_name;
      }

    private:
      func_name m_name;
      value_list m_params;
      value_list m_args;
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

  void call_stack::push(const sym_t& callable, const args_t& args, vis_t& visitor) noexcept
  {
    utils::unused(callable, args, visitor);
    //auto&& cur = m_frames.emplace_back(callable.name());
    //cur.allocate(callable.param_count());
    //auto&& argStorage = cur.args();
    //auto&& prmStorage = cur.params();

    //for (auto [param, arg] : utils::make_iterators(callable.params(), args))
    //{
    //  auto prmVal = param->symbol().value();
    //  auto argVal = arg->value();
    //  auto&& storedArg = argStorage.emplace_back();
    //  auto&& storedPrm = prmStorage.emplace_back();
    //  storedPrm = visitor.visit_assign(&storedPrm, prmVal);
    //  storedArg = visitor.visit_assign(&storedArg, argVal);
    //}
  }

  void call_stack::pop(vis_t& visitor) noexcept
  {
    if (m_frames.empty())
      return;

    auto&& cur = m_frames.back();
    for (auto [param, arg] : utils::make_iterators(cur.params(), cur.args()))
    {
      visitor.remove_entity(&param);
      visitor.remove_entity(&arg);
    }

    m_frames.pop_back();
  }

  void call_stack::prologue(sym_t& callable, vis_t& visitor) noexcept
  {
    if (m_frames.empty())
    {
      UTILS_ASSERT(false);
      return;
    }

    auto&& top = m_frames.back();
    for (auto [param, arg] : utils::make_iterators(callable.params(), top.args()))
    {
      auto&& sym = param->symbol();
      auto storedVal = visitor.visit_assign(&sym, arg);
      sym.eval_result(storedVal);
    }
  }

  void call_stack::epilogue(sym_t& callable, vis_t& visitor) noexcept
  {
    if (m_frames.empty())
    {
      return;
    }

    auto&& top = m_frames.back();
    for (auto [param, storedPrm] : utils::make_iterators(callable.params(), top.params()))
    {
      auto&& sym = param->symbol();
      auto restoredVal = visitor.visit_assign(&sym, storedPrm);
      sym.eval_result(restoredVal);
    }

    pop(visitor);
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