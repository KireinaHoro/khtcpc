#include <boost/asio.hpp>
#include <iostream>

#include "mgmt.h"

int main() {
  boost::asio::io_context ctx;
  auto conn = khtcpc::mgmt::connect_to_server(ctx);

  for (int i = 1; i < 2; ++i) {
    std::cout << khtcpc::mgmt::find_device(
                     conn, (std::string("eth") + std::to_string(i)).c_str())
              << std::endl;
  }

  for (;;)
    sleep(1);
}