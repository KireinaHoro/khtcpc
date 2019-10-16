#include <boost/asio.hpp>
#include <iomanip>
#include <iostream>

#include "client-request.h"
#include "ip.h"
#include "mgmt.h"

int main(int argc, char **argv) {
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

  if (argc != 3) {
    fprintf(stderr, "usage: %s <src-ip> <dst-ip>\n", argv[0]);
    return -1;
  }
  boost::asio::io_context ctx;
  auto conn = khtcpc::mgmt::connect_to_server();

  struct sockaddr_in src, dst;
  if (inet_pton(AF_INET, argv[1], &src.sin_addr) != 1) {
    fprintf(stderr, "error: src-addr %s is not a valid IP address.\n", argv[1]);
    return -1;
  }
  if (inet_pton(AF_INET, argv[2], &dst.sin_addr) != 1) {
    fprintf(stderr, "error: dst-addr %s is not a valid IP address.\n", argv[2]);
    return -1;
  }

  std::thread t([]() { khtcpc::mgmt::run(); });
  khtcpc::ip::async_write(
      conn, 78, src, dst, 0, 255, boost::asio::buffer(payload, strlen(payload)),
      [&](const struct khtcpc::response *resp_ptr, const void *payload_ptr) {
        BOOST_ASSERT(!payload_ptr);
        if (resp_ptr->ip_write.ret) {
          std::cerr << "Failed to write IP packet: Errno "
                    << resp_ptr->ip_write.ret;
        } else {
          std::cout << "IP packet written." << std::endl;
        }
        khtcpc::mgmt::finalize();
      });

  t.join();
}