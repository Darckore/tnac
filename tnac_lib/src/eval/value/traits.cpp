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
}