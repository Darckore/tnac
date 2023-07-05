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

      value_list& allocate(size_type count) noexcept
      {
        m_args.clear();
        m_args.reserve(count);
        return m_args;
      }

      func_name name() const noexcept
      {
        return m_name;
      }

    private:
      func_name m_name;
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
    auto&& cur = m_frames.emplace_back(callable.name());
    auto&& storage = cur.allocate(callable.param_count());

    for (auto arg : args)
    {
      auto argVal = arg->value();
      auto&& paramVal = storage.emplace_back();
      paramVal = visitor.visit_assign(&paramVal, argVal);
    }
  }

  void call_stack::pop(vis_t& visitor) noexcept
  {
    if (m_frames.empty())
      return;

    for (auto&& val : m_frames.back().args())
    {
      visitor.remove_entity(&val);
    }

    m_frames.pop_back();
  }

  void call_stack::prologue(sym_t& callable) noexcept
  {
    if (m_frames.empty())
    {
      UTILS_ASSERT(false);
      return;
    }

    set_params(callable);
  }

  void call_stack::epilogue(sym_t& callable) noexcept
  {
    if (!m_frames.empty())
    {
      set_params(callable);
      return;
    }

    for (auto param : callable.params())
    {
      param->symbol().eval_result({});
    }
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

  void call_stack::set_params(sym_t& callable) noexcept
  {
    auto&& top = m_frames.back();
    for (auto [param, arg] : utils::make_iterators(callable.params(), top.args()))
    {
      param->symbol().eval_result(arg);
    }
  }

}