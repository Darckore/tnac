#include "eval/value/traits.hpp"
#include "eval/value/type_impl.hpp"
#include "eval/value/value_store.hpp"

namespace tnac::eval::detail
{
  array_type to_array(store& vs, value val) noexcept
  {
    if (val.id() == type_id::Array)
      return val.get<array_type>();

    using sz = store::size_type;
    auto&& arr = vs.allocate_array(sz{ 1 });
    arr.add(std::move(val));
    return array_type{ vs.wrap(arr) };
  }
}

namespace tnac::eval
{
  value head(const array_type& arr) noexcept
  {
    if (arr->begin() == arr->end())
      return value{};

    return *arr->begin();
  }

  value tail(const array_type& op) noexcept
  {
    auto arr = op;
    auto&& wrapper = arr.wrapper();
    using sz = array_data::size_type;
    constexpr auto threshold = sz{ 2u };
    const auto size = wrapper.size();
    if (size < threshold)
      return value{};
    else if (size == threshold)
      return *std::next(wrapper.begin());

    auto&& data = arr->data();
    auto&& vs = arr->val_store();
    auto&& res = vs.wrap(data, wrapper.offset() + 1, size - 1u);
    return value{ array_type{ res } };
  }

  template <eval::expr_result Obj, typename Int, Int... Seq>
  inline val_opt instantiate(eval::cval_array<sizeof...(Seq)>& args, utils::idx_seq<Int, Seq...>) noexcept
  {
    using type_info = eval::type_info<Obj>;
    using type_gen = eval::type_wrapper<Obj>;
    auto instance = type_gen{}(
      eval::cast_value<utils::id_to_type_t<type_info::params[Seq]>>(args[Seq])...);

    return instance ? val_opt{ value{ *instance } } : val_opt{};
  }

  template <eval::expr_result Obj>
  inline val_opt instantiate(std::size_t argSz, arg_filler filler) noexcept
  {
    static constexpr auto max = eval::type_info<Obj>::maxArgs;
    val_array<max> args{};
    UTILS_ASSERT(argSz <= max);
    for (std::size_t idx{}; idx < argSz; ++idx)
      filler(args[idx]);

    return instantiate<Obj>(args, utils::idx_gen<max>{});
  }

  val_opt instantiate(type_id ti, std::size_t argSz, arg_filler filler) noexcept
  {
    using enum eval::type_id;
    switch (ti)
    {
    case Bool:     return instantiate<eval::bool_type>(argSz, std::move(filler));
    case Int:      return instantiate<eval::int_type>(argSz, std::move(filler));
    case Float:    return instantiate<eval::float_type>(argSz, std::move(filler));
    case Complex:  return instantiate<eval::complex_type>(argSz, std::move(filler));
    case Fraction: return instantiate<eval::fraction_type>(argSz, std::move(filler));
    }

    return val_opt{};
  }

  array_wrapper* extract_array(const value& val) noexcept
  {
    auto arrOpt = cast_value<array_type>(val);
    return arrOpt ? &(arrOpt->wrapper()) : nullptr;
  }
}