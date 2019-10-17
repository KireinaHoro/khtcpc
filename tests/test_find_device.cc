#include <boost/asio.hpp>
#include <iostream>

#include "mgmt.h"

int main() {
  for (int i = 0; i < 5; ++i) {
    std::cout << khtcpc::mgmt::find_device(
                     (std::string("eth") + std::to_string(i)).c_str())
              << std::endl;
  }

  khtcpc::mgmt::finalize();

  khtcpc::mgmt::run();
}