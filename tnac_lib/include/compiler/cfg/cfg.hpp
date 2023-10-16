//
// CFG
//

#pragma once
#include "compiler/cfg/func.hpp"
#include "eval/value/value_registry.hpp"
#include "eval/value/value_visitor.hpp"

namespace tnac::comp
{
  //
  // Control-flow graph for the program
  //
  class cfg final
  {
  public:
    using storage_key = utils::hashed_string;

    using func_name = func::name_t;
    using func_store = std::unordered_map<storage_key, func>;

    using op_t = basic_block::op_t;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(cfg);

    ~cfg() noexcept;
    cfg() noexcept;

  public: // Functions
    //
    // Creates a new function
    //
    func& create_function(func_name name) noexcept;

    //
    // Attempts to find a function by name
    //
    func* find_func(storage_key name) noexcept;

    //
    // Exits the currently processed function
    //
    void end_function() noexcept;

  public: // Expressions
    //
    // Consumes a unary expression
    //
    void consume_unary(eval::val_ops opCode) noexcept;

    //
    // Consumes a binary expression
    //
    void consume_binary(eval::val_ops opCode) noexcept;

  public: // Literals
    //
    // Consumes an integer literal
    //
    void consume_int(string_t raw, int base) noexcept;

    //
    // Consumes a float literal
    //
    void consume_float(string_t raw) noexcept;

    //
    // Consumes true
    //
    void consume_true() noexcept;

    //
    // Consumes false
    //
    void consume_false() noexcept;

    //
    // Consumes imaginary unit
    //
    void consume_i() noexcept;

    //
    // Consumes the e constant
    //
    void consume_e() noexcept;

    //
    // Consumes the pi constant
    //
    void consume_pi() noexcept;

  private:
    //
    // Emits a constant instuction
    //
    void emit_constant() noexcept;

    //
    // Returns a reference to the currently processed function
    //
    func& current_func() noexcept;

    //
    // Returns a reference to the current basic block
    //
    basic_block& current_block() noexcept;

    //
    // Returns a reference to the current environment
    //
    environment& env() noexcept;

  private:
    func* m_currentFunction{};
    func_store m_functions;
    eval::registry m_valReg;
    eval::value_visitor m_valVisitor;
  };
}