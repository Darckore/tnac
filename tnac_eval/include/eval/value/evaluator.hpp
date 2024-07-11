//
// Evaluator
//

#pragma once
#include "eval/value/value_registry.hpp"

namespace tnac::eval
{
  //
  // Evaluator for expressions with known values
  //
  class evaluator final
  {
  public:
    using arr_t      = registry::val_array;
    using size_type  = registry::size_type;

  public:
    CLASS_SPECIALS_NONE(evaluator);

    ~evaluator() noexcept;

    explicit evaluator(registry& reg) noexcept;

  private: // Unary operations
    void unary_plus(plusable auto operand) noexcept;
    void unary_plus(expr_result auto) noexcept;

    void unary_neg(negatable auto operand) noexcept;
    void unary_neg(expr_result auto) noexcept;

    void bitwise_not(expr_result auto operand) noexcept;

    void logical_not(expr_result auto operand) noexcept;

    void logical_is(expr_result auto operand) noexcept;

    void abs(abs_compatible auto operand) noexcept;
    void abs(expr_result auto) noexcept;

  private: // Binary operations
    // Bitwise

    void bitwise_and(expr_result auto lhs, expr_result auto rhs) noexcept;

    void bitwise_xor(expr_result auto lhs, expr_result auto rhs) noexcept;

    void bitwise_or(expr_result auto lhs, expr_result auto rhs) noexcept;

    // Simple arithmetic

    void add(addable auto lhs, addable auto rhs) noexcept;
    void add(expr_result auto, expr_result auto) noexcept;

    void sub(subtractable auto lhs, subtractable auto rhs) noexcept;
    void sub(expr_result auto, expr_result auto) noexcept;

    void mul(multipliable auto lhs, multipliable auto rhs) noexcept;
    void mul(expr_result auto, expr_result auto) noexcept;

    void div(divisible auto lhs, divisible auto rhs) noexcept;
    void div(expr_result auto, expr_result auto) noexcept;

    // Modulo

    void mod(fmod_divisible auto lhs, fmod_divisible auto rhs) noexcept;
    void mod(modulo_divisible auto lhs, modulo_divisible auto rhs) noexcept;
    void mod(expr_result auto, expr_result auto) noexcept;

    // Pow and root

    template <expr_result T> requires (std::is_arithmetic_v<T>)
    auto enforce_complex(const T& l, const T& r) noexcept -> typed_value<complex_type>;
    auto enforce_complex(const expr_result auto&, const expr_result auto&) noexcept;

    template <expr_result T> requires (std::is_arithmetic_v<T>)
    auto neg_root(const T& l, const T& r) noexcept -> typed_value<float_type>;
    auto neg_root(const expr_result auto&, const expr_result auto&) noexcept;

    void power(pow_raisable auto base, pow_raisable auto exp) noexcept;
    void power(expr_result auto base, expr_result auto exp) noexcept;

    void root(invertible auto base, invertible auto exp) noexcept;
    void root(expr_result auto, expr_result auto) noexcept;

    // Relation and equality

    void equal(eq_comparable auto lhs, eq_comparable auto rhs, bool compareForEquality) noexcept;
    void equal(array_type lhs, array_type rhs, bool compareForEquality) noexcept;
    void equal(expr_result auto, expr_result auto, bool) noexcept;

    void less(rel_comparable auto lhs, rel_comparable auto rhs) noexcept;
    void less(array_type lhs, array_type rhs) noexcept;
    void less(expr_result auto, expr_result auto) noexcept;

    void less_eq(fully_comparable auto lhs, fully_comparable auto rhs) noexcept;
    void less_eq(array_type lhs, array_type rhs) noexcept;
    void less_eq(expr_result auto, expr_result auto) noexcept;

    void greater(fully_comparable auto lhs, fully_comparable auto rhs) noexcept;
    void greater(array_type lhs, array_type rhs) noexcept;
    void greater(expr_result auto, expr_result auto) noexcept;

    void greater_eq(fully_comparable auto lhs, fully_comparable auto rhs) noexcept;
    void greater_eq(array_type lhs, array_type rhs) noexcept;
    void greater_eq(expr_result auto, expr_result auto) noexcept;

  private: // Operations
    //
    // Registers result of unary operations
    //
    template <expr_result T, unary_function<T> F>
    void visit_unary(T val, F&& op) noexcept;

    //
    // Dispatches unary operations according to operator type
    //
    template <expr_result T>
    void visit_unary(T operand, val_ops op) noexcept;

    template <>
    void visit_unary(array_type operand, val_ops op) noexcept;

    //
    // Registers result of binary operations
    //
    template <expr_result L, expr_result R, binary_function<L, R> F>
    void visit_binary(L lhs, R rhs, F&& op) noexcept;

    //
    // Dispatches binary operations according to operator type
    //
    template <expr_result L, expr_result R>
    void visit_binary(L l, R r, val_ops op) noexcept;

    template <>
    void visit_binary(array_type l, array_type r, val_ops op) noexcept;

    template <expr_result T> requires (!utils::same_noquals<T, array_type>)
    void visit_binary(array_type l, T r, val_ops op) noexcept;

    template <expr_result T> requires (!utils::same_noquals<T, array_type>)
    void visit_binary(T l, array_type r, val_ops op) noexcept;

  private:
    //
    // Extracts type from value and calls the specified function
    //
    template <typename F>
    void visit_value(value val, F&& func) noexcept;

    template <expr_result T>
    auto to_unit_array(const T& val) noexcept;

    //
    // Registers the value in the registry
    //
    void reg_value(expr_result auto val) noexcept;

    //
    // Registers result of assignment operations
    //
    void visit_assign(expr_result auto rhs) noexcept;

    //
    // Intermediate binary visitor
    // Dispatches the right operand according to its type
    //
    void visit_binary(expr_result auto lhs, value rhs, val_ops op) noexcept;

    //
    // Fills the specified number of arg values in the given container
    //
    void fill_args(value_container auto& args, size_type count) noexcept;

    //
    // Dispatches the instantiation call
    //
    template <expr_result Obj, typename T, T... Seq>
    void instantiate(const std::array<stored_value, sizeof...(Seq)>& args, std::integer_sequence<T, Seq...>) noexcept;

    //
    // Fills the specified number of args from the stack and instantiates an object
    //
    template <expr_result Obj>
    void instantiate(size_type argSz) noexcept;

  public:
    //
    // Instantiates an object using the specified number of args
    //
    void instantiate(type_id type, size_type argSz) noexcept;

    //
    // Stores a value for a function
    //
    value make_function(entity_id ent, function_type f) noexcept;

    //
    // Evaluates a binary expr
    //
    void visit_binary(value lhs, value rhs, val_ops op) noexcept;

    //
    // Evaluates a unary expr
    //
    void visit_unary(value val, val_ops op) noexcept;

    //
    // Pushes the given value to the stack
    //
    void push_value(value val) noexcept;

    //
    // Pushes the last result to the stack
    //
    void push_last() noexcept;

    //
    // Makes a value for an assigned-to entity
    //
    value visit_assign(entity_id ent, value rhs) noexcept;

    //
    // Parses an integer literal value from string
    //
    void visit_int_literal(string_t src, int base) noexcept;

    //
    // Parses a floating point literal value from string
    //
    void visit_float_literal(string_t src) noexcept;

    //
    // Registers a boolean literal
    //
    void visit_bool_literal(bool value) noexcept;

    //
    // Registers an imaginary 'i' value
    //
    void visit_i() noexcept;

    //
    // Registers the value of pi
    //
    void visit_pi() noexcept;

    //
    // Registers the value of pi
    //
    void visit_e() noexcept;

    //
    // Retrieves the next value from the stack of temporaries
    //
    stored_value fetch_next() noexcept;

    //
    // Retrieves the last evaluation result
    //
    value last() noexcept;

    //
    // Resets the last evaluation result and returns an empty value
    //
    void clear_result() noexcept;

  private:
    entity_id m_curEntity{};
    registry& m_registry;
  };
}