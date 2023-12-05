#include "driver/entry.hpp"
#include "driver/driver.hpp"

#if TNAC_WINDOWS
  #define WIN32_LEAN_AND_MEAN
  #define NOGDICAPMASKS
  #define NOMENUS
  #define NOKEYSTATES
  #define NOSYSCOMMANDS
  #define OEMRESOURCE
  #define NOATOM
  #define NOCLIPBOARD
  #define NOCOLOR
  #define NOCTLMGR
  #define NODRAWTEXT
  #define NONLS
  #define NOMB
  #define NOMEMMGR
  #define NOMETAFILE
  #define NOMINMAX
  #define NOOPENFILE
  #define NOSCROLL
  #define NOSERVICE
  #define NOSOUND
  #define NOTEXTMETRIC
  #define NOWH
  #define NOCOMM
  #define NOKANJI
  #define NOHELP
  #define NOPROFILER
  #define NODEFERWINDOWPOS
  #define NOMCX
  #include <windows.h>
  #include <windowsx.h>
  #include <eh.h>
#else
  //TNAC_BAD_PLATFORM
#endif

#if TNAC_WINDOWS
  #define TNAC_RUN(ARGC, ARGV) __try { init_console(); go(ARGC, ARGV); } \
                               __except (EXCEPTION_EXECUTE_HANDLER) { on_seh(GetExceptionCode()); }
#else
  #define TNAC_RUN(ARGC, ARGV) go(ARGC, ARGV)
#endif

namespace tnac::rt
{
  namespace
  {
    void on_terminate() noexcept
    {
      std::cerr << "std::terminate called\n";
    } 
    void on_failed_alloc() noexcept
    {
      std::cerr << "memory allocation failed\n";
    }
  
    struct handler_reg final
    {
      CLASS_SPECIALS_NONE_CUSTOM(handler_reg);
  
      handler_reg() noexcept
      {
        utils::set_new(on_failed_alloc);
        utils::set_terminate(-1, on_terminate);
      }
    };
  
    void set_terminate_handlers() noexcept
    {
      static handler_reg _{};
    }

    void go(int argc, char** argv) noexcept
    {
      set_terminate_handlers();
      driver drv{ argc, argv };
    }

#if TNAC_WINDOWS
    void init_console()
    {
      auto console = GetStdHandle(STD_OUTPUT_HANDLE);
      if (console == INVALID_HANDLE_VALUE)
        return;

      if (DWORD mode = 0; GetConsoleMode(console, &mode))
      {
        SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        std::cout << "\x1b]0;tnac\x1b\x5c";
      }
    }

    [[noreturn]] void on_seh(unsigned err) noexcept
    {
      std::cerr << "Fatal error " << std::hex << err << '\n';
      std::terminate();
    }
#endif
  }

  void execute(int argc, char** argv) noexcept
  {
    TNAC_RUN(argc, argv);
  }
}