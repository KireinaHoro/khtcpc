/**
 * @file eth.h
 * @author Pengcheng Xu <jsteward@pku.edu.cn>
 * @brief IP functions.
 * @version 0.1
 * @date 2019-10-07
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef __KHTCPC_IP_H_
#define __KHTCPC_IP_H_

#include <boost/log/trivial.hpp>

#include "client-request.h"
#include "mgmt.h"
#include "types.h"

namespace khtcpc {
namespace ip {
template <typename MutableBufferSequence>
void async_write(server_conn &sock, uint8_t proto,
                 const struct sockaddr_in &src, const struct sockaddr_in &dst,
                 uint8_t dscp, uint8_t ttl,
                 const MutableBufferSequence &payload_buf, handler_t &&handler,
                 check_buffer<MutableBufferSequence> = 0) {
  static struct request req;
  static struct response resp;
  req.type = IP_WRITE;
  req.id = rand();
  req.payload_len = payload_buf.size();

  req.ip_write.proto = proto;
  req.ip_write.dscp = dscp;
  req.ip_write.ttl = ttl;
  memcpy(&req.ip_write.src, &src, sizeof(src));
  memcpy(&req.ip_write.dst, &dst, sizeof(dst));

  boost::asio::write(sock, boost::asio::buffer(&req, sizeof(req)));
  boost::asio::write(sock, payload_buf);
  mgmt::get_pending_map()[req.id] = std::move(handler);
  mgmt::wait_response(sock);
}

void async_read(server_conn &sock, uint8_t proto, handler_t &&handler);
} // namespace ip
} // namespace khtcpc

#endif