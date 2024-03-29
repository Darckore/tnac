#include "driver/state.hpp"

namespace tnac::rt
{
  // Special members

  state::~state() noexcept = default;

  state::state(core& tnacCore) noexcept :
    m_tnac{ &tnacCore }
  {}


  // Public members

  core& state::tnac_core() noexcept
  {
    return *m_tnac;
  }

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

  int state::num_base() const noexcept
  {
    return m_numBase;
  }
  void state::reset_base() noexcept
  {
    m_numBase = defaultBase;
  }
  void state::set_base(int base) noexcept
  {
    m_numBase = base;
  }

  bool state::redirect_to_file(const fsys::path& to) noexcept
  {
    m_outFile.open(to);
    if (!m_outFile)
      return false;

    m_out = &m_outFile;
    return true;
  }
  void state::reset_output() noexcept
  {
    if (m_outFile)
      m_outFile.close();

    m_out = &std::cout;
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