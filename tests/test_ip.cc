#include <boost/asio.hpp>
#include <iomanip>
#include <iostream>

#include "client-request.h"
#include "ip.h"
#include "mgmt.h"

int main() {
  char payload[] =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
      "tempor incididunt ut labore et dolore magna aliqua. Nisi quis eleifend "
      "quam adipiscing. Pellentesque eu tincidunt tortor aliquam. In eu mi "
      "bibendum neque. Augue lacus viverra vitae congue eu consequat ac. "
      "Habitasse platea dictumst quisque sagittis purus. Semper risus in "
      "hendrerit gravida rutrum quisque non tellus orci. Eu mi bibendum neque "
      "egestas congue. Quam nulla porttitor massa id neque aliquam. Maecenas "
      "accumsan lacus vel facilisis. Ultrices in iaculis nunc sed augue lacus "
      "viverra vitae congue. Nulla facilisi morbi tempus iaculis urna id "
      "volutpat. Eget mauris pharetra et ultrices neque ornare. Aliquam id "
      "diam maecenas ultricies. Sit amet purus gravida quis blandit turpis. Ac "
      "tincidunt vitae semper quis lectus nulla at. Lacus suspendisse faucibus "
      "interdum posuere lorem ipsum dolor. Scelerisque purus semper eget duis "
      "at tellus. Auctor urna nunc id cursus metus aliquam eleifend. Cursus "
      "metus aliquam eleifend mi in nulla posuere. Ut tortor pretium viverra "
      "suspendisse potenti nullam ac tortor vitae. Quisque sagittis purus sit "
      "amet volutpat consequat mauris. In iaculis nunc sed augue. Mus mauris "
      "vitae ultricies leo integer malesuada nunc. Aliquet lectus proin nibh "
      "nisl. Vulputate dignissim suspendisse in est ante in nibh mauris. "
      "Sapien et ligula ullamcorper malesuada proin libero nunc.";
  boost::asio::io_context ctx;
  int dev = khtcpc::mgmt::find_device("eth0");
  if (dev < 0) {
    std::cerr << "Device eth0 not found" << std::endl;
    exit(-1);
  }

  int num_ip;
  auto src = khtcpc::mgmt::get_device_ip(dev, &num_ip);
  if (num_ip < 1) {
    fprintf(stderr, "interface eth0 does not have IP address.\n");
    return -1;
  }
  struct sockaddr_in dst;
  dst.sin_addr.s_addr = 0x0185a8c0; // 192.168.133.1
  dst.sin_family = AF_INET;

  // test read TCP
  khtcpc::ip::async_read(6, [&](const struct khtcpc::response *resp_ptr,
                                const void *payload_ptr) {
    BOOST_ASSERT(payload_ptr);
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &resp_ptr->ip_read.src.sin_addr, ip_str,
              INET_ADDRSTRLEN);
    std::cout << "IP " << ip_str << " > ";
    inet_ntop(AF_INET, &resp_ptr->ip_read.dst.sin_addr, ip_str,
              INET_ADDRSTRLEN);
    std::cout << ip_str << ", dscp " << (int)resp_ptr->ip_read.dscp
              << std::endl;
    std::cout << "IP payload length " << resp_ptr->payload_len
              << ", content: " << std::endl;
    for (int i = 0; i < resp_ptr->payload_len; ++i) {
      if (i % 16 == 0) {
        std::cout << std::hex << std::setw(8) << std::setfill('0') << i << ": ";
      }
      std::cout << std::hex << std::setw(2) << std::setfill('0')
                << (int)((const uint8_t *)payload_ptr)[i];
      if (i % 16 == 15) {
        std::cout << std::endl;
      } else if (i % 2 == 1) {
        std::cout << ' ';
      }
    }
    std::cout << std::endl;
  });

  khtcpc::ip::async_write(
      78, *src, dst, 0, 255, boost::asio::buffer(payload, strlen(payload)),
      [&](const struct khtcpc::response *resp_ptr, const void *payload_ptr) {
        BOOST_ASSERT(!payload_ptr);
        if (resp_ptr->ip_write.ret < 0) {
          std::cerr << "Failed to write IP packet: Errno "
                    << resp_ptr->ip_write.ret;
        } else {
          std::cout << "IP packet written." << std::endl;
        }
        khtcpc::mgmt::finalize();
      });

  khtcpc::mgmt::run();

  free(src);
}