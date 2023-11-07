//
// Runtime state
//

#pragma once

namespace tnac::rt
{
  //
  // State shared between the driver and REPL
  //
  class state final
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(state);

    ~state() noexcept;

    state() noexcept;

  public:
    bool is_running() const noexcept;
    void start() noexcept;
    void stop() noexcept;

  public: // IO
    in_stream&  in() noexcept;
    out_stream& out() noexcept;
    out_stream& err() noexcept;

  private:
    in_stream*  m_in{ &std::cin };
    out_stream* m_out{ &std::cout };
    out_stream* m_err{ &std::cerr };

    bool m_running{};
  };
}