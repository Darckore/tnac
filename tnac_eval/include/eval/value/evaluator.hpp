//
// Evaluator
//

#pragma once
#include "eval/value/value_registry.hpp"

namespace tnac::eval::detail
{
  template <typename T>
  concept value_container = requires (T& t)
  {
    { t.operator[](0) } -> std::same_as<stored_value&>;
  };
}

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
    void unary_plus(detail::plusable auto operand) noexcept;
    void unary_plus(detail::expr_result auto) noexcept;

    void unary_neg(detail::negatable auto operand) noexcept;
    void unary_neg(detail::expr_result auto) noexcept;

    void bitwise_not(detail::expr_result auto operand) noexcept;

    void logical_not(detail::expr_result auto operand) noexcept;

    void logical_is(detail::expr_result auto operand) noexcept;

    void abs(detail::abs_compatible auto operand) noexcept;
    void abs(detail::expr_result auto) noexcept;

  private: // Binary operations
    // Bitwise

    void bitwise_and(detail::expr_result auto lhs, detail::expr_result auto rhs) noexcept;

    void bitwise_xor(detail::expr_result auto lhs, detail::expr_result auto rhs) noexcept;

    void bitwise_or(detail::expr_result auto lhs, detail::expr_result auto rhs) noexcept;

    // Simple arithmetic

    void add(detail::addable auto lhs, detail::addable auto rhs) noexcept;
    void add(detail::expr_result auto, detail::expr_result auto) noexcept;

    void sub(detail::subtractable auto lhs, detail::subtractable auto rhs) noexcept;
    void sub(detail::expr_result auto, detail::expr_result auto) noexcept;

    void mul(detail::multipliable auto lhs, detail::multipliable auto rhs) noexcept;
    void mul(detail::expr_result auto, detail::expr_result auto) noexcept;

    void div(detail::divisible auto lhs, detail::divisible auto rhs) noexcept;
    void div(detail::expr_result auto, detail::expr_result auto) noexcept;

    // Modulo

    void mod(detail::fmod_divisible auto lhs, detail::fmod_divisible auto rhs) noexcept;
    void mod(detail::modulo_divisible auto lhs, detail::modulo_divisible auto rhs) noexcept;
    void mod(detail::expr_result auto, detail::expr_result auto) noexcept;

    // Pow and root

    template <detail::expr_result T> requires (std::is_arithmetic_v<T>)
    auto enforce_complex(const T& l, const T& r) noexcept -> typed_value<complex_type>;
    auto enforce_complex(const detail::expr_result auto&, const detail::expr_result auto&) noexcept;

    template <detail::expr_result T> requires (std::is_arithmetic_v<T>)
    auto neg_root(const T& l, const T& r) noexcept -> typed_value<float_type>;
    auto neg_root(const detail::expr_result auto&, const detail::expr_result auto&) noexcept;

    void power(detail::pow_raisable auto base, detail::pow_raisable auto exp) noexcept;
    void power(detail::expr_result auto base, detail::expr_result auto exp) noexcept;

    void root(detail::invertible auto base, detail::invertible auto exp) noexcept;
    void root(detail::expr_result auto, detail::expr_result auto) noexcept;

    // Relation and equality

    void equal(detail::eq_comparable auto lhs, detail::eq_comparable auto rhs, bool compareForEquality) noexcept;
    void equal(array_type lhs, array_type rhs, bool compareForEquality) noexcept;
    void equal(detail::expr_result auto, detail::expr_result auto, bool) noexcept;

    void less(detail::rel_comparable auto lhs, detail::rel_comparable auto rhs) noexcept;
    void less(array_type lhs, array_type rhs) noexcept;
    void less(detail::expr_result auto, detail::expr_result auto) noexcept;

    void less_eq(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept;
    void less_eq(array_type lhs, array_type rhs) noexcept;
    void less_eq(detail::expr_result auto, detail::expr_result auto) noexcept;

    void greater(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept;
    void greater(array_type lhs, array_type rhs) noexcept;
    void greater(detail::expr_result auto, detail::expr_result auto) noexcept;

    void greater_eq(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept;
    void greater_eq(array_type lhs, array_type rhs) noexcept;
    void greater_eq(detail::expr_result auto, detail::expr_result auto) noexcept;

  private: // Operations
    //
    // Registers result of unary operations
    //
    template <detail::expr_result T, detail::unary_function<T> F>
    void visit_unary(T val, F&& op) noexcept;

    //
    // Dispatches unary operations according to operator type
    //
    template <detail::expr_result T>
    void visit_unary(T operand, val_ops op) noexcept;

    template <>
    void visit_unary(array_type operand, val_ops op) noexcept;

    //
    // Registers result of binary operations
    //
    template <detail::expr_result L, detail::expr_result R, detail::binary_function<L, R> F>
    void visit_binary(L lhs, R rhs, F&& op) noexcept;

    //
    // Dispatches binary operations according to operator type
    //
    template <detail::expr_result L, detail::expr_result R>
    void visit_binary(L l, R r, val_ops op) noexcept;

    template <>
    void visit_binary(array_type l, array_type r, val_ops op) noexcept;

    template <detail::expr_result T> requires (!utils::same_noquals<T, array_type>)
    void visit_binary(array_type l, T r, val_ops op) noexcept;

    template <detail::expr_result T> requires (!utils::same_noquals<T, array_type>)
    void visit_binary(T l, array_type r, val_ops op) noexcept;

  private:
    //
    // Extracts type from value and calls the specified function
    //
    template <typename F>
    void visit_value(value val, F&& func) noexcept;

    template <detail::expr_result T>
    auto to_unit_array(const T& val) noexcept;

    //
    // Registers the value in the registry
    //
    void reg_value(detail::expr_result auto val) noexcept;

    //
    // Registers result of assignment operations
    //
    void visit_assign(detail::expr_result auto rhs) noexcept;

    //
    // Intermediate binary visitor
    // Dispatches the right operand according to its type
    //
    void visit_binary(detail::expr_result auto lhs, value rhs, val_ops op) noexcept;

    //
    // Dispatches the instantiation call
    //
    template <detail::expr_result Obj, typename T, T... Seq>
    void instantiate(const std::array<stored_value, sizeof...(Seq)>& args, std::integer_sequence<T, Seq...>) noexcept;

  public:
    //
    // Locks the given ref counted value
    //
    auto lock(const detail::lockable auto& val) noexcept
    {
      return value_lock{ val, m_registry };
    }

    //
    // Instantiates an object
    //
    template <detail::expr_result Obj, typename... Args> requires utils::all_same<stored_value, Args...>
    void instantiate(Args ...args) noexcept
    {
      using type_info = eval::type_info<Obj>;
      static constexpr auto max = type_info::maxArgs;
      static_assert(sizeof ...(Args) == max);

      const std::array argList{ std::move(args)... };
      instantiate<Obj>(argList, std::make_index_sequence<max>{});
    }

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

    //
    // Fills the specified number of arg values in the given container
    //
    void fill_args(detail::value_container auto& args, size_type count) noexcept
    {
      for (auto idx = count; idx > size_type{}; --idx)
      {
        args[idx - 1] = fetch_next();
      }
    }

    //
    // Creates and registers an array
    //
    void make_array(size_type count) noexcept;

    //
    // Creates a temporary local array and returns it without pushing it into
    // the registry. The result is pre-filled with args from the current stack
    // This is required for call expressions
    //
    arr_t collect_args_locally(size_type count) noexcept;

  private:
    entity_id m_curEntity{};
    registry& m_registry;
  };
}