//
// A convenient way to init everything the evaluator needs
//

#pragma once
#include "evaluator/evaluator.hpp"
#include "exec/value/value_registry.hpp"
#include "exec/call_stack.hpp"

namespace tnac::packages
{
  //
  // A wrapper for all things evaluator
  //
  class evaluator final
  {
  public:
    using size_type = tnac::eval::call_stack::size_type;

  public:
    CLASS_SPECIALS_NONE(evaluator);

    ~evaluator() noexcept;

    explicit evaluator(size_type stackSize) noexcept;

  public:
    //
    // Redirects to tnac::evaluator::operator()
    //
    void operator()(ast::node* root) noexcept;

    //
    // Last evaluation result
    //
    eval::value last_result() const noexcept;

  public:
    //
    // tnac::evaluator::on_error
    //
    template <eval::detail::err_handler F>
    void on_error(F&& handler) noexcept
    {
      m_evaluator.on_error(std::forward<F>(handler));
    }

  private:
    tnac::eval::registry m_registry;
    tnac::eval::call_stack m_callStack;
    tnac::evaluator m_evaluator;
  };
}