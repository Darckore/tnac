//
// Sema
//

#pragma once
#include "sema/sym_table.hpp"

namespace tnac
{
  namespace detail
  {
    template <typename F>
    concept var_handler = std::is_nothrow_invocable_r_v<void, F, const semantics::variable&>;
  }

  //
  // Semantic analyser. Constrols scope tracking and is responsible
  // for registering and looking up symbols
  //
  class sema final
  {
  public:
    using symbol = semantics::symbol;
    using sym_ptr = symbol*;
    
    using fake_name_set = std::unordered_set<buf_t>;
    
    using var_handler_t = std::move_only_function<void(const semantics::variable&) noexcept>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(sema);

    ~sema() noexcept;
    sema() noexcept;

  public:
    //
    // Sets a callback invoked when a variable is declared
    //
    template <detail::var_handler F>
    void on_variable(F&& f) noexcept
    {
      m_varCallback = std::forward<F>(f);
    }

    //
    // Opens a new scope
    //
    void open_scope() noexcept;

    //
    // Closes the current scope and makes its parent the current one
    //
    void close_scope() noexcept;

    //
    // Checks whether the specified symbol has previously been defined
    // and returns it if it was
    //
    sym_ptr find(string_t name, bool currentOnly = false) noexcept;

    //
    // Visits a newly created declarator and registers it in the symbol table
    //
    void visit_decl(ast::decl& decl) noexcept;

    //
    // Generates a random name
    //
    token contrive_name() noexcept;

  private:
    semantics::sym_table m_symTab;
    fake_name_set m_generatedNames;
    const semantics::scope* m_curScope{};
    var_handler_t m_varCallback{};
  };
}