#include "test_cases/test_common.hpp"

namespace tnac::tests
{
  TEST(refcounted, t_arr_wrappers)
  {
    eval::store store;
    auto&& arr = store.allocate_array(10ull);
    for (eval::int_type i = 0; i < 10; ++i)
      arr.add(eval::value{ i });

    auto&& wrap1 = store.wrap(arr);
    auto&& wrap2 = store.wrap(arr, 2ull);
    auto&& wrap3 = store.wrap(arr, 3ull, 2ull);

    auto&& arrlist = arr.list();
    auto&& wrplist = wrap1.list();

    wrplist.remove(wrap1);
    ASSERT_FALSE(arrlist.empty());

    wrplist.remove(wrap2);
    ASSERT_TRUE(arr.is_last());

    wrplist.remove(wrap3);
    ASSERT_TRUE(arrlist.empty());
  }
}