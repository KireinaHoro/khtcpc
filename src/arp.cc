#include <boost/log/trivial.hpp>

#include "arp.h"
#include "mgmt.h"

namespace khtcpc {
namespace arp {
using namespace boost::asio;
void async_write(int dev_id, int opcode, struct sockaddr_ll &sender_mac,
                 struct sockaddr_in &sender_ip, struct sockaddr_ll &target_mac,
                 struct sockaddr_in &target_ip, handler_t &&handler) {
  static struct request req;
  static struct response resp;
  req.type = ARP_WRITE;
  req.id = rand();
  req.payload_len = 0;

  BOOST_LOG_TRIVIAL(trace) << "Sending ARP write request #" << req.id;

  req.arp_write.dev_id = dev_id;
  req.arp_write.opcode = opcode;
  memcpy(&req.arp_write.sender_mac, &sender_mac, sizeof(sender_mac));
  memcpy(&req.arp_write.target_mac, &target_mac, sizeof(target_mac));
  memcpy(&req.arp_write.sender_ip, &sender_ip, sizeof(sender_ip));
  memcpy(&req.arp_write.target_ip, &target_ip, sizeof(target_ip));

  boost::asio::write(mgmt::get_conn(), boost::asio::buffer(&req, sizeof(req)));
  mgmt::get_pending_map()[req.id] = std::move(handler);
  mgmt::wait_response();
}

void async_read(int dev_id, handler_t &&handler) {
  static struct request req;
  static struct response resp;
  req.type = ARP_READ;
  req.id = rand();
  req.payload_len = 0;

  BOOST_LOG_TRIVIAL(trace) << "Sending ARP read request #" << req.id;

  req.arp_read.dev_id = dev_id;

  boost::asio::write(mgmt::get_conn(), boost::asio::buffer(&req, sizeof(req)));
  mgmt::get_pending_map()[req.id] = std::move(handler);
  mgmt::wait_response();
}
} // namespace arp
} // namespace khtcpc