/**
 * @file socket.h
 * @author Pengcheng Xu <jsteward@pku.edu.cn>
 * @brief Socket functions.
 * @version 0.1
 * @date 2019-10-18
 *
 * @copyright Copyright (c) 2019
 *
 */
#ifndef __KHTCPC_SOCKET_H_
#define __KHTCPC_SOCKET_H_

#include "client-request.h"

#include <functional>

namespace khtcpc {
namespace socket {
/**
 * @brief (ret)
 */
using sendto_handler = std::function<void(ssize_t)>;
/**
 * @brief (ret, payload_ptr, payload_len)
 */
using recvfrom_handler = std::function<void(ssize_t, const void *, uint64_t)>;
int socket(int type);
int close(int fd);
int bind(int fd, struct sockaddr_in *addr);
void async_sendto(int fd, const void *message, size_t length,
                  struct sockaddr_in *dst, sendto_handler &&handler);
void async_recvfrom(int fd, struct sockaddr_in *src,
                    recvfrom_handler &&handler);
} // namespace socket
} // namespace khtcpc

#endif
