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