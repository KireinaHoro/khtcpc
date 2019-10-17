#include <boost/asio.hpp>
#include <iomanip>
#include <iostream>

#include "client-request.h"
#include "eth.h"
#include "mgmt.h"

int main() {
  char payload[] =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
      "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
      "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
      "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
      "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
      "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
      "mollit anim id est laborum.";

  int dev = khtcpc::mgmt::find_device("eth0");
  struct sockaddr_ll mac;
  memcpy(&mac.sll_addr, "\xff\xff\xff\xff\xff\xff", 6);
  mac.sll_halen = 6;

  khtcpc::eth::async_write(
      dev, 0xdead, mac, boost::asio::buffer(payload, strlen(payload)),
      [&](const struct khtcpc::response *resp_ptr, const void *payload_ptr) {
        BOOST_ASSERT(!payload_ptr);
        BOOST_ASSERT(resp_ptr->eth_write.dev_id == dev);
        std::cout << "Frame written." << std::endl;
      });

  khtcpc::eth::async_read(dev, [&](const struct khtcpc::response *resp_ptr,
                                   const void *payload_ptr) {
    BOOST_ASSERT(payload_ptr);
    BOOST_ASSERT(resp_ptr->eth_read.dev_id == dev);
    std::cout << "Frame EtherType: 0x" << std::hex << std::setw(4)
              << std::setfill('0') << resp_ptr->eth_read.ethertype << std::endl;
    std::cout << "Frame content: " << std::endl;
    for (int i = 0; i < resp_ptr->payload_len; ++i) {
      std::cout << std::hex << std::setw(2) << std::setfill('0')
                << (int)((const uint8_t *)payload_ptr)[i];
      if (i % 16 == 15) {
        std::cout << std::endl;
      } else if (i % 2 == 1) {
        std::cout << ' ';
      }
    }
    std::cout << std::endl;
    khtcpc::mgmt::finalize();
  });

  khtcpc::mgmt::run();
}