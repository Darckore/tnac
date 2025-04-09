#include "eval/traits.hpp"
#include "eval/type_impl.hpp"
#include "eval/value_store.hpp"

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
    if (wrapper.size() == array_data::size_type{})
      return value{};

    auto&& data = arr->data();
    auto&& vs = arr->val_store();
    auto&& res = vs.wrap(data, wrapper.offset() + 1, wrapper.size() - 1u);
    return value{ array_type{ res } };
  }
}