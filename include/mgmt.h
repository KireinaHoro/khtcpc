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

void wait_response(server_conn &conn);

void read_response_handler(const boost::system::error_code &ec, int bytes,
                           server_conn &sock, struct response &response_);

server_conn connect_to_server(boost::asio::io_context &io_context);

int find_device(server_conn &conn, const char *name);

void get_device_mac(server_conn &conn, int dev_id, struct sockaddr_ll *mac_o);

struct sockaddr_in *get_device_ip(server_conn &conn, int dev_id, int *number_o);

} // namespace mgmt
} // namespace khtcpc

#endif