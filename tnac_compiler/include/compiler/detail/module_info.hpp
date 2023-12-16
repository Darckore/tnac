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

  namespace ir
  {
    class function;
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

    //
    // Sets the given module as the current one
    //
    void enter_module(ir::function& mod) noexcept;

    //
    // Resets the current module to nothing
    //
    void exit_module() noexcept;

    //
    // Returns a pointer to the current module
    //
    ir::function* try_current_module() noexcept;

    //
    // Checks whether a current module is set
    //
    bool has_current_module() const noexcept;

    //
    // Returns a reference to the current module
    // Use with caution and check whether one exists beforehand
    //
    ir::function& current_module() noexcept;

  private:
    data_store m_data;
    module_stack m_stack;
    ir::function* m_curModule{};
  };
}