#include "driver/driver.hpp"

int main()
{
  tnac_rt::driver driver;
  driver.run_interactive();

  using ct = tnac::common_type_t<int, float>;

  return 0;
}