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
    using module_sym = semantics::module_sym;
    using module_def = ast::module_def;
    using data_store = std::unordered_map<module_sym*, module_def*>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(module_info);

    ~module_info() noexcept;

    module_info() noexcept;

  public:
    //
    // Appends a sym-tree association
    // Returns true if the module was encountered for the first time
    //
    void store(module_sym& sym, module_def& def) noexcept;

    //
    // Returns the AST node corresponding to a module
    //
    module_def* locate(module_sym& sym) noexcept;

  private:
    data_store m_data;
  };
}