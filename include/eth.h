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
void async_write(int dev_id, uint16_t ethertype, const struct sockaddr_ll &mac,
                 const MutableBufferSequence &payload_buf, handler_t &&handler,
                 check_buffer<MutableBufferSequence> = 0) {
  auto *req = (struct request *)malloc(sizeof(struct request));
  auto *resp = (struct response *)malloc(sizeof(struct response));
  req->type = ETHERNET_WRITE;
  req->id = rand();
  req->payload_len = payload_buf.size();

  req->eth_write.dev_id = dev_id;
  req->eth_write.ethertype = ethertype;
  memcpy(&req->eth_write.mac, &mac, sizeof(mac));

  boost::asio::write(mgmt::get_conn(),
                     boost::asio::buffer(req, sizeof(struct request)));
  boost::asio::write(mgmt::get_conn(), payload_buf);
  mgmt::wait_response(req, resp, std::move(handler));
}

void async_read(int dev_id, handler_t &&handler);
} // namespace eth
} // namespace khtcpc

#endif