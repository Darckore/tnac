#include "common/diag.hpp"

namespace tnac::detail
{
  static constexpr auto wrongArgN{ "Too {} arguments. Expected {}, got {}"sv };

  static constexpr auto expected{ "Expected {}"sv };
  static constexpr auto expectedCh{ "Expected '{}'"sv };
  static constexpr auto expectedInfo{ "Expected {} {}"sv };
  static constexpr auto expectedInfoCh{ "Expected '{}' {}"sv };
  static constexpr auto expectedAfter{ "Expected {} after {}"sv };
  static constexpr auto expectedAfterCh{ "Expected '{}' after {}"sv };

  static constexpr auto failedImport{ "Unable to import module '{}'"sv };
  static constexpr auto circularImport{ "Circular reference between modules '{}' and '{}'"sv };
  static constexpr auto selfImport{ "Illegal self import in module '{}'"sv };

  static constexpr auto wrongCmdArgType{ "Command argument {} has an unexpected type"sv };
  static constexpr auto wrongCmdArg{ "Unrecognised argument '{}' at index {}"sv };

  static constexpr auto unknownCliArg{ "Unknown cli arg '{}'"sv };
  static constexpr auto fileErr{ "Failed to {} file '{}'. Reason: '{}'"sv };

  static constexpr auto moduleError{ "Compilation stopped due to errors in module '{}'"sv };

  static constexpr auto conditionSame{ "The condition is always {}"sv };
  static constexpr auto logicalSame{ "The logical '{}' expression is always {}. Check the {} operand"sv };
}

namespace tnac
{
  // Data members

  utils::string_pool diag::m_interned{};


  // Public members

  string_t diag::expected(string_t what) noexcept
  {
    return format(detail::expected, what);
  }

  string_t diag::expected(char_t what) noexcept
  {
    return format(detail::expectedCh, what);
  }

  string_t diag::expected(string_t what, string_t more) noexcept
  {
    return format(detail::expectedInfo, what, more);
  }

  string_t diag::expected(char_t what, string_t more) noexcept
  {
    return format(detail::expectedInfoCh, what, more);
  }
  
  
  // Public members(Compile)

  string_t diag::wrong_arg_num(size_type wanted, size_type got) noexcept
  {
    static constexpr auto few { "few"sv };
    static constexpr auto many{ "many"sv };
    auto quantity = wanted < got ? many : few;
    return format(detail::wrongArgN, quantity, wanted, got);
  }

  string_t diag::compilation_stopped() noexcept
  {
    return "Compilation stopped due to errors"sv;
  }

  string_t diag::compilation_stopped(string_t moduleName) noexcept
  {
    return format(detail::moduleError, moduleName);
  }

  string_t diag::condition_same(bool value) noexcept
  {
    return format(detail::conditionSame, value);
  }

  string_t diag::logical_same(string_t op, bool isLhs, bool value) noexcept
  {
    const auto operand = isLhs ? "left"sv : "right"sv;
    return format(detail::logicalSame, op, value, operand);
  }

  string_t diag::unreachable() noexcept
  {
    return "Unreachable code"sv;
  }

  string_t diag::ret_here() noexcept
  {
    return "The related ret expression is here"sv;
  }

  string_t diag::all_branches_return() noexcept
  {
    return "All branches of the conditional expression return"sv;
  }


  // Public members(Parsing)

  string_t diag::expected_expr() noexcept
  {
    return expected("expression"sv);
  }

  string_t diag::expected_args() noexcept
  {
    return expected("argument list"sv);
  }

  string_t diag::expected_assignable() noexcept
  {
    return expected("an assignable object"sv);
  }

  string_t diag::expected_id() noexcept
  {
    return expected("identifier"sv);
  }

  string_t diag::expected_init() noexcept
  {
    return expected("initialisation"sv);
  }

  string_t diag::expected_single_id() noexcept
  {
    return expected("a single identifier"sv);
  }

  string_t diag::expected_expr_sep() noexcept
  {
    return expected(':', "or EOL"sv);
  }

  string_t diag::expected_func_end() noexcept
  {
    return format(detail::expectedAfterCh, ';', "function definition"sv);
  }

  string_t diag::expected_cond_end() noexcept
  {
    return format(detail::expectedAfterCh, ';', "conditional"sv);
  }

  string_t diag::expected_pattern_end() noexcept
  {
    return format(detail::expectedAfterCh, ';', "pattern body"sv);
  }

  string_t diag::expected_matcher_def() noexcept
  {
    return format(detail::expectedAfterCh, "->"sv, "condition matcher"sv);
  }

  string_t diag::undef_id() noexcept
  {
    return "Undefined identifier"sv;
  }

  string_t diag::scope_ref_nodot() noexcept
  {
    return "References to intermediate scopes can only be used as lhs of a dot expression"sv;
  }

  string_t diag::invalid_decl() noexcept
  {
    return "Invalid declaration"sv;
  }

  string_t diag::invalid_lambda() noexcept
  {
    return "Invalid anonimous function definition"sv;
  }

  string_t diag::param_redef() noexcept
  {
    return "Function parameter redifinition"sv;
  }

  string_t diag::name_redef() noexcept
  {
    return "Name redifinition"sv;
  }

  string_t diag::expr_not_allowed() noexcept
  {
    return "Expression is not allowed here"sv;
  }

  string_t diag::empty_cond() noexcept
  {
    return "Condition can't be empty"sv;
  }

  string_t diag::empty_import() noexcept
  {
    return "Expected import name"sv;
  }

  string_t diag::import_failed(string_t name) noexcept
  {
    return format(detail::failedImport, name);
  }

  string_t diag::circular_ref(string_t last, string_t cur) noexcept
  {
    return format(detail::circularImport, last, cur);
  }

  string_t diag::self_import(string_t moduleName) noexcept
  {
    return format(detail::selfImport, moduleName);
  }

  string_t diag::lit_after_dot() noexcept
  {
    return "Literal is not allowed as an accessor after '.'"sv;
  }


  // Public members(Commands)

  string_t diag::unknown_cmd() noexcept
  {
    return "Unknown command"sv;
  }

  string_t diag::wrong_cmd_arg(size_type idx) noexcept
  {
    return format(detail::wrongCmdArgType, idx + 1);
  }

  string_t diag::wrong_cmd_arg(size_type idx, string_t value) noexcept
  {
    return format(detail::wrongCmdArg, value, idx + 1);
  }
  
  // Public members(Runtime)

  string_t diag::unknown_cli_arg(string_t arg) noexcept
  {
    return format(detail::unknownCliArg, arg);
  }

  string_t diag::file_load_failure(const fsys::path& path, string_t reason) noexcept
  {
    return format(detail::fileErr, "load"sv, path.string(), reason);
  }

  string_t diag::file_write_failure(const fsys::path& path, string_t reason) noexcept
  {
    return format(detail::fileErr, "write to"sv, path.string(), reason);
  }

}