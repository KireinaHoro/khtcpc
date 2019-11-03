#include <arpa/inet.h>
#include <iomanip>
#include <iostream>

#include "client-request.h"
#include "mgmt.h"
#include "socket.h"

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
      "nisl. Vun nibh mauris. Sapien et ligula ullamcorper malesuada proin "
      "libero nunc.";

  bool do_bind = false;
  struct sockaddr_in src, dst;
  if (argc == 5) {
    do_bind = true;
    if (inet_pton(AF_INET, argv[3], &src.sin_addr) != 1) {
      fprintf(stderr, "error: %s is not a valid IP address.\n", argv[3]);
      return -1;
    }
    src.sin_family = AF_INET;
    src.sin_port = htons(atoi(argv[4]));
  } else if (argc != 3) {
    fprintf(stderr, "usage: %s <dst-ip> <dst-port> [src-ip] [src-port]\n",
            argv[0]);
    return -1;
  }
  if (inet_pton(AF_INET, argv[1], &dst.sin_addr) != 1) {
    fprintf(stderr, "error: %s is not a valid IP address.\n", argv[1]);
    return -1;
  }
  dst.sin_family = AF_INET;
  dst.sin_port = htons(atoi(argv[2]));

  std::thread t([]() { khtcpc::mgmt::run(); });

  using namespace khtcpc::socket;
  int fd = socket(SOCK_DGRAM);
  if (fd < 0) {
    std::cerr << "Failed to create socket, Errno: " << fd << std::endl;
    return -1;
  }

  if (do_bind) {
    int ret;
    if ((ret = bind(fd, &src)) < 0) {
      std::cerr << "Failed to bind socket, Errno: " << ret << std::endl;
      return -1;
    }
  }

  async_sendto(fd, payload, sizeof(payload), &dst, [fd](ssize_t ret) {
    if (ret < 0) {
      std::cerr << "Failed to send UDP packet, Errno: " << ret << std::endl;
    } else {
      std::cout << "Successfully written " << ret << " bytes in UDP."
                << std::endl;
    }
    khtcpc::socket::close(fd);
    khtcpc::mgmt::finalize();
  });

  t.join();
}