#include "driver/state.hpp"

namespace tnac::rt
{
  // Special members

  state::~state() noexcept = default;

  state::state() noexcept = default;


  // Public members(IO)

  in_stream& state::in() noexcept
  {
    return *m_in;
  }
  out_stream& state::out() noexcept
  {
    return *m_out;
  }
  out_stream& state::err() noexcept
  {
    return *m_err;
  }
}