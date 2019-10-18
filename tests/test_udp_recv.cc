#include <iomanip>
#include <iostream>

#include "client-request.h"
#include "mgmt.h"
#include "socket.h"

int main(int argc, char **argv) {
  struct sockaddr_in bind_addr;
  if (argc != 3) {
    fprintf(stderr, "usage: %s <bind-ip> <bind-port>\n", argv[0]);
    return -1;
  }
  if (inet_pton(AF_INET, argv[1], &bind_addr.sin_addr) != 1) {
    fprintf(stderr, "error: %s is not a valid IP address.\n", argv[1]);
    return -1;
  }
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_port = atoi(argv[2]);

  std::thread t([]() { khtcpc::mgmt::run(); });

  using namespace khtcpc::socket;
  int fd = socket(SOCK_DGRAM);
  if (fd < 0) {
    std::cerr << "Failed to create socket, Errno: " << fd << std::endl;
    return -1;
  }
  bind(fd, &bind_addr);

  async_recvfrom(fd, [fd](ssize_t ret, struct sockaddr_in src,
                          const void *payload_ptr, uint64_t payload_len) {
    char ipstring[INET_ADDRSTRLEN];
    inet_ntop(src.sin_family, &src.sin_addr, ipstring, INET_ADDRSTRLEN);
    std::cout << "UDP payload length " << payload_len << " from " << ipstring
              << ":" << src.sin_port << ", content: " << std::endl;
    for (int i = 0; i < payload_len; ++i) {
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
    khtcpc::socket::close(fd);
    khtcpc::mgmt::finalize();
  });

  t.join();
}