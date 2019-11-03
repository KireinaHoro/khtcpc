#include "socket.h"
#include "mgmt.h"

#include <boost/asio.hpp>

namespace khtcpc {
namespace socket {
using namespace boost::asio;
int socket(int type) {
  struct request req;
  struct response resp;
  req.type = SOCKET;
  req.id = rand();
  req.socket.type = type;
  write(mgmt::get_conn(), buffer(&req, sizeof(req)));
  read(mgmt::get_conn(), buffer(&resp, sizeof(resp)));

  BOOST_ASSERT(resp.type == SOCKET);
  BOOST_ASSERT(resp.id == req.id);
  return resp.socket.fd;
}

int close(int fd) {
  struct request req;
  struct response resp;
  req.type = CLOSE;
  req.id = rand();
  req.close.fd = fd;
  write(mgmt::get_conn(), buffer(&req, sizeof(req)));
  read(mgmt::get_conn(), buffer(&resp, sizeof(resp)));

  BOOST_ASSERT(resp.type == CLOSE);
  BOOST_ASSERT(resp.id == req.id);
  return resp.close.ret;
}

int bind(int fd, const struct sockaddr_in *addr) {
  struct request req;
  struct response resp;
  req.type = BIND;
  req.id = rand();
  req.bind.fd = fd;
  memcpy(&req.bind.addr, addr, sizeof(struct sockaddr_in));
  write(mgmt::get_conn(), buffer(&req, sizeof(req)));
  read(mgmt::get_conn(), buffer(&resp, sizeof(resp)));

  BOOST_ASSERT(resp.type == BIND);
  BOOST_ASSERT(resp.id == req.id);
  return resp.bind.ret;
}

void async_sendto(int fd, const void *message, size_t length,
                  const struct sockaddr_in *dst, sendto_handler &&handler) {
  auto *req = (struct request *)malloc(sizeof(struct request));
  auto *resp = (struct response *)malloc(sizeof(struct response));
  req->type = SENDTO;
  req->id = rand();
  req->payload_len = length;

  req->sendto.fd = fd;
  memcpy(&req->sendto.dst, dst, sizeof(struct sockaddr_in));

  write(mgmt::get_conn(), buffer(req, sizeof(struct request)));
  write(mgmt::get_conn(), buffer(message, length));
  mgmt::wait_response(req, resp,
                      [=](const response *resp, const void *payload_ptr) {
                        BOOST_ASSERT(resp->type == SENDTO);
                        BOOST_ASSERT(resp->id == req->id);
                        BOOST_ASSERT(!payload_ptr);
                        handler(resp->sendto.ret);
                      });
}

void async_recvfrom(int fd, recvfrom_handler &&handler) {
  auto *req = (struct request *)malloc(sizeof(struct request));
  auto *resp = (struct response *)malloc(sizeof(struct response));
  req->type = RECVFROM;
  req->id = rand();
  req->payload_len = 0;

  req->recvfrom.fd = fd;

  write(mgmt::get_conn(), buffer(req, sizeof(struct request)));
  mgmt::wait_response(
      req, resp, [req, handler](const response *resp, const void *payload_ptr) {
        BOOST_ASSERT(resp->type == RECVFROM);
        BOOST_ASSERT(resp->id == req->id);
        BOOST_ASSERT(payload_ptr);
        handler(resp->recvfrom.ret, resp->recvfrom.src, payload_ptr,
                resp->payload_len);
      });
}
} // namespace socket
} // namespace khtcpc
