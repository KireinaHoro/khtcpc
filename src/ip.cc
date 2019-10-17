#include "ip.h"

namespace khtcpc {
namespace ip {
void async_read(uint8_t proto, handler_t &&handler) {
  auto *req = (struct request *)malloc(sizeof(struct request));
  auto *resp = (struct response *)malloc(sizeof(struct response));
  req->type = IP_READ;
  req->id = rand();
  req->payload_len = 0;

  req->ip_read.proto = proto;

  boost::asio::write(mgmt::get_conn(),
                     boost::asio::buffer(req, sizeof(struct request)));
  mgmt::wait_response(req, resp, std::move(handler));
}
} // namespace ip
} // namespace khtcpc