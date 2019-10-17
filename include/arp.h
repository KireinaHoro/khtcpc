/**
 * @file arp.h
 * @author Pengcheng Xu <jsteward@pku.edu.cn>
 * @brief ARP functions.
 * @version 0.1
 * @date 2019-10-07
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef __KHTCPC_ARP_H_
#define __KHTCPC_ARP_H_

#include <boost/asio.hpp>

#include "client-request.h"
#include "types.h"

namespace khtcpc {
namespace arp {
void async_write(int dev_id, int opcode, struct sockaddr_ll &sender_mac,
                 struct sockaddr_in &sender_ip, struct sockaddr_ll &target_mac,
                 struct sockaddr_in &target_ip, handler_t &&handler);
void async_read(int dev_id, handler_t &&handler);
} // namespace arp
} // namespace khtcpc

#endif