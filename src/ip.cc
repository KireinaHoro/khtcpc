#include "ip.h"

namespace khtcpc {
namespace ip {
void async_read(uint8_t proto, handler_t &&handler) {
  static struct request req;
  static struct response resp;
  req.type = IP_READ;
  req.id = rand();
  req.payload_len = 0;

  req.ip_read.proto = proto;

  boost::asio::write(mgmt::get_conn(), boost::asio::buffer(&req, sizeof(req)));
  mgmt::get_pending_map()[req.id] = std::move(handler);
  mgmt::wait_response();
}
} // namespace ip
} // namespace khtcpc