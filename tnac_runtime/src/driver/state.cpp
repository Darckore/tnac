#include "driver/state.hpp"

namespace tnac::rt
{
  // Special members

  state::~state() noexcept = default;

  state::state() noexcept = default;


  // Public members

  bool state::is_running() const noexcept
  {
    return m_running;
  }
  void state::start() noexcept
  {
    m_running = true;
  }
  void state::stop() noexcept
  {
    m_running = false;
  }


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