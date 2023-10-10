//
// CFG
//

#pragma once
#include "compiler/cfg/basic_block.hpp"
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
    using block_name  = basic_block::name_t;
    using block_store = std::unordered_map<storage_key, basic_block>;
    using entry_stack = std::stack<basic_block*>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(cfg);

    ~cfg() noexcept;
    cfg() noexcept;

  public:
    //
    // Enters a basic block
    //
    void enter_block(basic_block& block) noexcept;

    //
    // Exits a basic block and rolls back to the one
    // entered before
    //
    void exit_block() noexcept;

    //
    // Creates a new basic block
    //
    basic_block& create(block_name name) noexcept;

    //
    // Returns a reference to the entry block
    //
    basic_block& entry() noexcept;

    //
    // Attempts to find a basic block by name
    //
    basic_block* find(storage_key name) noexcept;

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
    basic_block* m_entry{};
    block_store m_blocks;
    entry_stack m_entryChain;
    eval::registry m_valReg;
    eval::value_visitor m_valVisitor;
  };
}