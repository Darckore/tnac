//
// Control flow graph
//

#pragma once
#include "cfg/ir/ir_builder.hpp"

namespace tnac::ir
{
  //
  // Control flow graph of the program
  // Provides access to the IR
  //
  class cfg final
  {
  public:
    using name_t      = function::name_t;
    using module_list = std::vector<function*>;
    using const_store = std::vector<eval::stored_value>;
    using const_view  = std::span<const eval::stored_value>;
    using size_type   = module_list::size_type;
    using instr_list  = builder::instruction_list;
    using edge_list   = builder::edge_list;

  public:
    CLASS_SPECIALS_NONE(cfg);

    ~cfg() noexcept;

    explicit cfg(builder& bld) noexcept;

  public:
    //
    // Returns a reference to the IR builder
    //
    builder& get_builder() noexcept;

    //
    // Declares a new module
    //
    function& declare_module(entity_id id, name_t name, size_type paramCount) noexcept;

    //
    // Declares a new function
    //
    function& declare_function(entity_id id, function& owner, name_t name, size_type paramCount) noexcept;

    //
    // Returns a pointer to the module or function corresponding to the given id
    //
    function* find_entity(entity_id id) noexcept;

    //
    // Connects blocks with an edge
    //
    edge& connect(basic_block& from, basic_block& to, operand val) noexcept;

    //
    // Returns a reference to the instruction list
    //
    const instr_list& instructions() const noexcept;

    //
    // Returns a reference to the instruction list
    //
    instr_list& instructions() noexcept;

    //
    // Returns a reference to the edge list
    //
    const edge_list& edges() const noexcept;

    //
    // Returns a reference to the edge list
    //
    edge_list& edges() noexcept;

    //
    // Returns a view into the list of interned static constants
    //
    const_view interned_vals() const noexcept;

    //
    // Interns a value in the static const storage
    //
    void intern(eval::stored_value val) noexcept;

    //
    // Interns a value in the static const storage
    //
    void intern(eval::value val) noexcept;

  public:
    //
    // Returns a const begin iterator to the module collection
    //
    auto begin() const noexcept
    {
      return m_modules.cbegin();
    }

    //
    // Returns a begin iterator to the module collection
    //
    auto begin() noexcept
    {
      return m_modules.begin();
    }

    //
    // Returns a const end iterator to the module collection
    //
    auto end() const noexcept
    {
      return m_modules.cend();
    }

    //
    // Returns an end iterator to the module collection
    //
    auto end() noexcept
    {
      return m_modules.end();
    }

  private:
    //
    // Casts and returns a function's parameter count from size_type
    //
    static function::size_type conv_param_count(size_type paramCount) noexcept;

  private:
    builder* m_builder;
    module_list m_modules;
    const_store m_constants;
  };
}