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
    using size_type = std::size_t;

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

  public: // Compile
    //
    // Returns a 'wrong number of arguments' message
    //
    static string_t wrong_arg_num(size_type wanted, size_type got) noexcept;

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

    //
    // Returns an 'empty import name' message
    //
    static string_t empty_import() noexcept;

    //
    // Returns an 'import failed' message
    //
    static string_t import_failed(string_t name) noexcept;

  public: // Commands
    //
    // Returns an 'unknown command' message
    //
    static string_t unknown_cmd() noexcept;

    //
    // Returns a 'wrong cmd arg type' message
    //
    static string_t wrong_cmd_arg(size_type idx) noexcept;

    //
    // Returns a 'wrong cmd arg' message
    //
    static string_t wrong_cmd_arg(size_type idx, string_t value) noexcept;

  public: // Runtime
    //
    // Returns an 'unknown cli arg' message
    //
    static string_t unknown_cli_arg(string_t arg) noexcept;

    //
    // Returns a 'failed to load file' message
    //
    static string_t file_load_failure(const fsys::path& path, string_t reason) noexcept;

    //
    // Returns a 'failed to write to file' message
    //
    static string_t file_write_failure(const fsys::path& path, string_t reason) noexcept;

  private:
    static buf_t m_buffer;
    static msg_store m_interned;
  };
}