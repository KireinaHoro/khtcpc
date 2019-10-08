/**
 * @file eth.h
 * @author Pengcheng Xu <jsteward@pku.edu.cn>
 * @brief Ethernet functions.
 * @version 0.1
 * @date 2019-10-07
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef __KHTCPC_ETH_H_
#define __KHTCPC_ETH_H_

#include <boost/log/trivial.hpp>

#include "client-request.h"
#include "mgmt.h"
#include "types.h"

namespace khtcpc {
namespace eth {
template <typename MutableBufferSequence>
void async_write(server_conn &sock, int dev_id, uint16_t ethertype,
                 const struct sockaddr_ll &mac,
                 const MutableBufferSequence &payload_buf, handler_t &&handler,
                 check_buffer<MutableBufferSequence> = 0) {
  static struct request req;
  static struct response resp;
  req.type = ETHERNET_WRITE;
  req.id = rand();
  req.payload_len = payload_buf.size();

  req.eth_write.dev_id = dev_id;
  req.eth_write.ethertype = ethertype;
  memcpy(&req.eth_write.mac, &mac, sizeof(mac));

  boost::asio::write(sock, boost::asio::buffer(&req, sizeof(req)));
  boost::asio::write(sock, payload_buf);
  mgmt::get_pending_map()[req.id] = std::move(handler);
  mgmt::wait_response(sock);
}

void async_read(server_conn &sock, int dev_id, handler_t &&handler);
} // namespace eth
} // namespace khtcpc

#endif