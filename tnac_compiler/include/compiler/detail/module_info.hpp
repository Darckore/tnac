//
// Module info
//

#pragma once

namespace tnac
{
  namespace ast
  {
    class module_def;
  }

  namespace semantics
  {
    class module_sym;
  }
}

namespace tnac::detail
{
  //
  // Holds associations between module symbols and their corresponding ASTs
  //
  class module_info final
  {
  public:
    using module_sym   = semantics::module_sym;
    using module_def   = ast::module_def;
    using data_store   = std::unordered_map<module_sym*, module_def*>;
    using module_stack = std::vector<module_sym*>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(module_info);

    ~module_info() noexcept;

    module_info() noexcept;

  public:
    //
    // Appends a sym-tree association
    //
    void store(module_sym& sym, module_def& def) noexcept;

    //
    // Returns the AST node corresponding to a module
    //
    module_def* locate(module_sym& sym) noexcept;

    //
    // Discards the collected data
    //
    void wipe() noexcept;

    //
    // Pushes a module symbol on the stack
    //
    void push(module_sym& sym) noexcept;

    //
    // Pops the next module symbol from the stack
    // Returns nullptr if empty
    //
    module_sym* pop() noexcept;

  private:
    data_store m_data;
    module_stack m_stack;
  };
}