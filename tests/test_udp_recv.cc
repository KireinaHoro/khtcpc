#include <iomanip>
#include <iostream>

#include "client-request.h"
#include "mgmt.h"
#include "socket.h"

int main(int argc, char **argv) {
  struct sockaddr_in src;
  if (argc != 3) {
    fprintf(stderr, "usage: %s <src-ip> <src-port>\n", argv[0]);
    return -1;
  }
  if (inet_pton(AF_INET, argv[1], &src.sin_addr) != 1) {
    fprintf(stderr, "error: %s is not a valid IP address.\n", argv[1]);
    return -1;
  }
  src.sin_family = AF_INET;
  src.sin_port = atoi(argv[2]);

  std::thread t([]() { khtcpc::mgmt::run(); });

  using namespace khtcpc::socket;
  int fd = socket(SOCK_DGRAM);
  if (fd < 0) {
    std::cerr << "Failed to create socket, Errno: " << fd << std::endl;
    return -1;
  }

  async_recvfrom(
      fd, &src,
      [fd](ssize_t ret, const void *payload_ptr, uint64_t payload_len) {
        std::cout << "UDP payload length " << payload_len
                  << ", content: " << std::endl;
        for (int i = 0; i < payload_len; ++i) {
          if (i % 16 == 0) {
            std::cout << std::hex << std::setw(8) << std::setfill('0') << i
                      << ": ";
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