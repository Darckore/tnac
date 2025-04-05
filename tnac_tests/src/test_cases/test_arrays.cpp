#include "test_cases/test_common.hpp"

namespace tnac::tests
{
  namespace
  {
    using arr_data = eval::array_data;
    using arr_wrap = eval::array_wrapper;
    using arr_list = utils::ilist<arr_data>;
    using wrp_list = utils::ilist<arr_wrap>;

    struct arr_store
    {
      arr_list arrays;
      wrp_list wrappers;
    };
  }

  TEST(arrays, t_ref_counter)
  {
    arr_store as;

    auto&& arr = as.arrays.emplace_back(10ull);
    for (eval::int_type i = 0; i < 10; ++i)
      arr.add(eval::value{ i });

    auto&& wrap1 = as.wrappers.emplace_back(arr);
    auto&& wrap2 = as.wrappers.emplace_back(arr, 2ull);
    auto&& wrap3 = as.wrappers.emplace_back(arr, 3ull, 2ull);

    as.wrappers.remove(wrap1);
    ASSERT_FALSE(as.arrays.empty());

    as.wrappers.remove(wrap2);
    ASSERT_TRUE(arr.is_last());

    as.wrappers.remove(wrap3);
    ASSERT_TRUE(as.arrays.empty());
  }
}