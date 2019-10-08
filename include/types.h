/**
 * @file types.h
 * @author Pengcheng Xu <jsteward@pku.edu.cn>
 * @brief Concepts for types.
 * @version 0.1
 * @date 2019-10-08
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef __KHTCPC_TYPES_H_
#define __KHTCPC_TYPES_H_

#include <boost/asio.hpp>
#include <boost/type_traits.hpp>
#include <functional>
#include <unordered_map>

namespace khtcpc {

using server_conn = boost::asio::local::stream_protocol::socket;

template <typename MutableBufferSequence>
using check_buffer = typename boost::enable_if_c<
    boost::asio::is_mutable_buffer_sequence<MutableBufferSequence>::value>::type
    *;

// const response *: response_ptr
// const void *: payload_ptr
using handler_t = std::function<void(const response *, const void *)>;
using pending_map = std::unordered_map<int, handler_t>;

} // namespace khtcpc

#endif