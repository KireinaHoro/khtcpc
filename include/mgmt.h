/**
 * @file mgmt.h
 * @author Pengcheng Xu <jsteward@pku.edu.cn>
 * @brief Management functions.
 * @version 0.1
 * @date 2019-10-08
 *
 * @copyright Copyright (c) 2019
 *
 */
#ifndef __KHTCPC_MGMT_H_
#define __KHTCPC_MGMT_H_

#include <boost/bind.hpp>
#include <cstring>
#include <iostream>

#include "client-request.h"
#include "types.h"

namespace khtcpc {
namespace mgmt {

pending_map &get_pending_map();

void wait_response(struct request *req, struct response *resp,
                   handler_t &&handler);

void read_response_handler(const boost::system::error_code &ec, int bytes,
                           struct response &response_);

void connect_to_server();

int find_device(const char *name);

void get_device_mac(int dev_id, struct sockaddr_ll *mac_o);

struct sockaddr_in *get_device_ip(int dev_id, int *number_o);

server_conn &get_conn();

void finalize();
boost::asio::io_context &get_ctx();
void run();

} // namespace mgmt
} // namespace khtcpc

#endif