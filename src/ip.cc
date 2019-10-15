#include "ip.h"

namespace khtcpc {
namespace ip {
void async_read(server_conn &sock, int dev_id, uint8_t proto,
                handler_t &&handler) {
  static struct request req;
  static struct response resp;
  req.type = IP_READ;
  req.id = rand();
  req.payload_len = 0;

  req.ip_read.dev_id = dev_id;
  req.ip_read.proto = proto;

  boost::asio::write(sock, boost::asio::buffer(&req, sizeof(req)));
  mgmt::get_pending_map()[req.id] = std::move(handler);
  mgmt::wait_response(sock);
}
} // namespace ip
} // namespace khtcpc