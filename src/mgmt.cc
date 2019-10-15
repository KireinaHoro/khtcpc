#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <cstdlib>
#include <functional>
#include <queue>
#include <utility>

#include "mgmt.h"

namespace khtcpc {
namespace mgmt {

pending_map &get_pending_map() {
  static pending_map map;
  return map;
}

std::queue<
    std::pair<struct response *,
              std::function<void(const boost::system::error_code &, size_t)>>>
    read_response_handlers;

void wait_response(server_conn &conn) {
  using namespace boost::asio;
  struct response *response_ =
      (struct response *)malloc(sizeof(struct response));
  read_response_handlers.emplace(response_, [&, response_](const auto &ec,
                                                           auto bytes) {
    if (!ec) {
      read_response_handler(ec, bytes, conn, *response_);
      free(response_);
    } else {
      BOOST_LOG_TRIVIAL(error) << "Error in read response: " << ec.message();
    }
  });
}

bool finalized = false;
void finalize() { finalized = true; }

boost::asio::io_context ctx;
boost::asio::io_context &get_ctx() { return ctx; }
void run() { ctx.run(); }

server_conn connect_to_server() {
  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      boost::log::trivial::warning);
  using namespace boost::asio;
  local::stream_protocol::endpoint ep("/var/run/khtcp.sock");
  server_conn sock(ctx);
  sock.connect(ep);
  // init the random generator
  srand((unsigned)time(nullptr));

  // start the response handler processing loop
  static boost::asio::deadline_timer t(ctx);
  static std::function<void()> token = [&sock]() {
    if (!read_response_handlers.empty()) {
      auto &head = read_response_handlers.front();
      async_read(sock, boost::asio::buffer(head.first, sizeof(struct response)),
                 [&](const auto &ec, auto bytes) {
                   head.second(ec, bytes);
                   read_response_handlers.pop();
                   boost::asio::post(ctx, token);
                 });
    } else if (!finalized) {
      t.expires_from_now(boost::posix_time::millisec(1));
      t.async_wait([&](const auto &ec) { boost::asio::post(ctx, token); });
    }
  };
  boost::asio::post(ctx, token);

  return sock;
}

void read_response_handler(const boost::system::error_code &ec, int bytes,
                           server_conn &sock, struct response &response_) {
  using namespace boost::asio;
  try {
    void *payload_ptr = nullptr;
    auto &handler = get_pending_map().at(response_.id);
    if (response_.payload_len > 0) {
      payload_ptr = malloc(response_.payload_len);
      if (response_.payload_len !=
          read(sock, buffer(payload_ptr, response_.payload_len))) {
        BOOST_LOG_TRIVIAL(warning) << "Response payload short read";
      }
    }
    handler(&response_, payload_ptr);
    BOOST_LOG_TRIVIAL(trace) << "Called handler for request " << response_.id;
    get_pending_map().erase(response_.id);
    if (payload_ptr) {
      free(payload_ptr);
    }
  } catch (const std::out_of_range &) {
    BOOST_LOG_TRIVIAL(error) << "Unexpected response with id " << response_.id
                             << " received from server";
  }
}

int find_device(server_conn &conn, const char *name) {
  using namespace boost::asio;
  struct request req;
  struct response resp;
  req.type = FIND_DEVICE;
  req.id = rand();
  strncpy(req.find_device.name, name, IFNAMSIZE);
  write(conn, buffer(&req, sizeof(req)));
  read(conn, buffer(&resp, sizeof(resp)));

  BOOST_ASSERT(resp.type == FIND_DEVICE);
  BOOST_ASSERT(resp.id == req.id);
  return resp.find_device.dev_id;
}

void get_device_mac(server_conn &conn, int dev_id, struct sockaddr_ll *mac_o) {
  using namespace boost::asio;
  struct request req;
  struct response resp;
  req.type = GET_DEVICE_MAC;
  req.id = rand();
  req.get_device_mac.dev_id = dev_id;
  write(conn, buffer(&req, sizeof(req)));
  read(conn, buffer(&resp, sizeof(resp)));

  memcpy(mac_o, &resp.get_device_mac.mac, sizeof(*mac_o));
}

struct sockaddr_in *get_device_ip(server_conn &conn, int dev_id,
                                  int *number_o) {
  using namespace boost::asio;
  struct request req;
  struct response resp;
  req.type = GET_DEVICE_IP;
  req.id = rand();
  req.get_device_ip.dev_id = dev_id;

  write(conn, buffer(&req, sizeof(req)));
  read(conn, buffer(&resp, sizeof(resp)));

  *number_o = resp.get_device_ip.count;
  int size = *number_o * sizeof(struct sockaddr_in);
  struct sockaddr_in *ret = (struct sockaddr_in *)malloc(size);

  read(conn, buffer(ret, size));
  return ret;
}
} // namespace mgmt
} // namespace khtcpc