//
// Diagnostics
//

#pragma once

namespace tnac
{
  //
  // Stores error and othe diagnostic messages
  // and allows forming them
  //
  class diag final
  {
  public:
    using msg_store = std::unordered_map<utils::hashed_string, buf_t>;

  public:
    CLASS_SPECIALS_NONE(diag);

  public:
    //
    // Formats and interns a message using the given format string and args
    //
    template <typename ...Args>
    static string_t format(string_t fmt, Args&& ...args) noexcept
    {
      m_buffer.clear();
      std::vformat_to(std::back_inserter(m_buffer), fmt,
        std::make_format_args(std::forward<Args>(args)...));

      auto hs = utils::hashed_string{ m_buffer };
      auto interned = m_interned.try_emplace(hs, std::move(m_buffer));
      if (interned.second)
      {
        m_buffer = {};
      }

      return interned.first->second;
    }

    //
    // Formats and interns an 'expected X' kind of error
    //
    static string_t expected(string_t what) noexcept;

    //
    // Formats and interns an 'expected X' kind of error
    //
    static string_t expected(char_t what) noexcept;

    //
    // Formats and interns an 'expected X <SOMETHING>' kind of error
    //
    static string_t expected(string_t what, string_t more) noexcept;

    //
    // Formats and interns an 'expected X <SOMETHING>' kind of error
    //
    static string_t expected(char_t what, string_t more) noexcept;

  public: // Parsing
    //
    // Returns an 'expected expr' message
    //
    static string_t expected_expr() noexcept;

    //
    // Returns an 'expected arg list' message
    //
    static string_t expected_args() noexcept;

    //
    // Returns an 'expected arg list' message
    //
    static string_t expected_assignable() noexcept;

    //
    // Returns an 'expected init' message
    //
    static string_t expected_init() noexcept;

    //
    // Returns an 'expected identifier' message
    //
    static string_t expected_id() noexcept;

    //
    // Returns an 'expected single id' message
    //
    static string_t expected_single_id() noexcept;

    //
    // Returns a 'missing expression end character' error message
    //
    static string_t expected_expr_sep() noexcept;

    //
    // Returns a 'missing ; for function' error message
    //
    static string_t expected_func_end() noexcept;

    //
    // Returns a 'missing ; for conditional' error message
    //
    static string_t expected_cond_end() noexcept;

    //
    // Returns a 'missing ; for conditional' error message
    //
    static string_t expected_pattern_end() noexcept;

    //
    // Returns a 'missing -> for matcher' error message
    //
    static string_t expected_matcher_def() noexcept;

    //
    // Returns an 'undefined id' message
    //
    static string_t undef_id() noexcept;

    //
    // Returns an 'invalid decl' message
    //
    static string_t invalid_decl() noexcept;

    //
    // Returns an 'invalid anonimous function' message
    //
    static string_t invalid_lambda() noexcept;

    //
    // Returns a 'param redefinition' message
    //
    static string_t param_redef() noexcept;

    //
    // Returns an 'expression not allowed' message
    //
    static string_t expr_not_allowed() noexcept;

    //
    // Returns an 'empty condition' message
    //
    static string_t empty_cond() noexcept;

  private:
    static buf_t m_buffer;
    static msg_store m_interned;
  };
}