//
// Call stack for functions
//

#pragma once

namespace tnac::eval
{
  //
  // Manages the call stack
  //
  class call_stack final
  {
  public:
    using size_type = std::size_t;

  public:
    CLASS_SPECIALS_NONE(call_stack);

    ~call_stack() noexcept;

    explicit call_stack(size_type depth) noexcept;

    explicit operator bool() const noexcept;

  private:
    size_type m_depth{};
    bool m_overflown{};
  };
}