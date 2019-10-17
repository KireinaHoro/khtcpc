#include "eth.h"

namespace khtcpc {
namespace eth {
void async_read(int dev_id, handler_t &&handler) {
  auto *req = (struct request *)malloc(sizeof(struct request));
  auto *resp = (struct response *)malloc(sizeof(struct response));
  req->type = ETHERNET_READ;
  req->id = rand();
  req->payload_len = 0;

  req->eth_read.dev_id = dev_id;

  boost::asio::write(mgmt::get_conn(),
                     boost::asio::buffer(req, sizeof(struct request)));
  mgmt::get_pending_map()[req->id] = std::move(handler);
  mgmt::wait_response(req, resp);
}
} // namespace eth
} // namespace khtcpc