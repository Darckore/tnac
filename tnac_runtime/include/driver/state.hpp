//
// Runtime state
//

#pragma once
#include "core/tnac.hpp"
#include "output/formatting.hpp"

namespace tnac::rt
{
  //
  // State shared between the driver and REPL
  //
  class state final
  {
  public:
    static constexpr auto defaultBase = 10;

  public:
    CLASS_SPECIALS_NONE(state);

    ~state() noexcept;

    explicit state(core& tnacCore) noexcept;

  public:
    core& tnac_core() noexcept;

    bool is_running() const noexcept;
    void start() noexcept;
    void stop() noexcept;

    int num_base() const noexcept;
    void reset_base() noexcept;
    void set_base(int base) noexcept;

  public: // IO
    in_stream&  in() noexcept;
    out_stream& out() noexcept;
    out_stream& err() noexcept;

  private:
    core* m_tnac{};

    in_stream*  m_in{ &std::cin };
    out_stream* m_out{ &std::cout };
    out_stream* m_err{ &std::cerr };

    int  m_numBase{ defaultBase };
    bool m_running{};
  };
}