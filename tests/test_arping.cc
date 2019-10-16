#include <boost/asio.hpp>
#include <cstdio>
#include <string>

#include "arp.h"
#include "mgmt.h"

using namespace boost::asio;
using namespace khtcpc;

int count;
struct sockaddr_in *src_ip;
struct sockaddr_in *dst_ip;
struct sockaddr_ll *src_mac;
struct sockaddr_ll *dst_mac;
char *target_ip_str;
char *target_device_str;

void req_once(server_conn &conn, int dev) {
  static boost::posix_time::ptime sent_time;
  arp::async_write(
      conn, dev, 0x1, *src_mac, *src_ip, *dst_mac, *dst_ip,
      [&, dev](const struct response *resp_ptr, const void *payload_ptr) {
        BOOST_ASSERT(!payload_ptr);
        BOOST_ASSERT(resp_ptr->arp_write.dev_id == dev);
        sent_time = boost::posix_time::microsec_clock::local_time();
      });
  arp::async_read(
      conn, dev,
      [&, dev](const struct response *resp_ptr, const void *payload_ptr) {
        BOOST_ASSERT(!payload_ptr);
        BOOST_ASSERT(resp_ptr->arp_read.dev_id == dev);
        if (resp_ptr->arp_read.opcode == 0x2 &&
            !memcmp(resp_ptr->arp_read.target_mac.sll_addr, src_mac->sll_addr,
                    6)) {
          char str[INET_ADDRSTRLEN];
          inet_ntop(AF_INET, &resp_ptr->arp_read.sender_ip.sin_addr, str,
                    INET_ADDRSTRLEN);
          auto &m = resp_ptr->arp_read.sender_mac.sll_addr;
          printf(
              "Unicast reply from %s [%02x:%02x:%02x:%02x:%02x:%02x]  %.3fms\n",
              str, m[0], m[1], m[2], m[3], m[4], m[5],
              (float)(boost::posix_time::microsec_clock::local_time() -
                      sent_time)
                      .total_microseconds() /
                  1000);
        }
      });
}

void timer_handler(const boost::system::error_code &ec, server_conn &conn,
                   deadline_timer &t, int &counter, int dev) {
  if (!ec) {
    req_once(conn, dev);
    if (++counter < count) {
      t.expires_from_now(boost::posix_time::seconds(1));
      t.async_wait(boost::bind(timer_handler, placeholders::error,
                               boost::ref(conn), boost::ref(t),
                               boost::ref(counter), dev));
    } else {
      mgmt::finalize();
    }
  } else {
    fprintf(stderr, "timer: %s\n", ec.message().c_str());
  }
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: %s <interface> <target ip> <count>\n", argv[0]);
    return -1;
  }
  target_ip_str = argv[2];
  target_device_str = argv[1];
  try {
    count = std::stoi(std::string(argv[3]));
  } catch (std::exception) {
    fprintf(stderr, "%s is not a valid number.\n", argv[3]);
    return -1;
  }
  if (count < 1) {
    fprintf(stderr, "count needs to be greater than 0\n");
    return -1;
  }

  dst_ip = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  if (inet_pton(AF_INET, argv[2], &dst_ip->sin_addr) < 1) {
    fprintf(stderr, "%s is not a valid IP address.\n", argv[2]);
    return -1;
  }
  dst_mac = (struct sockaddr_ll *)malloc(sizeof(struct sockaddr_ll));
  memcpy(&dst_mac->sll_addr, "\xff\xff\xff\xff\xff\xff", 6);
  dst_mac->sll_halen = 6;

  auto conn = mgmt::connect_to_server();
  int dev = mgmt::find_device(conn, argv[1]);
  if (dev < 0) {
    fprintf(stderr, "failed to find interface %s.\n", argv[1]);
    return -1;
  }
  src_mac = (struct sockaddr_ll *)malloc(sizeof(struct sockaddr_ll));
  mgmt::get_device_mac(conn, dev, src_mac);

  int num_ip;
  auto ips = mgmt::get_device_ip(conn, dev, &num_ip);
  if (num_ip < 1) {
    fprintf(stderr, "interface %s does not have IP address.\n", argv[1]);
    return -1;
  }
  src_ip = &ips[0];

  char src_ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &src_ip->sin_addr, src_ip_str, INET_ADDRSTRLEN);

  printf("ARPING %s from %s %s\n", argv[2], src_ip_str, argv[1]);

  int counter = 1;
  req_once(conn, dev);
  deadline_timer t(mgmt::get_ctx());
  if (count > 1) {
    t.expires_from_now(boost::posix_time::seconds(1));
    t.async_wait(boost::bind(timer_handler, placeholders::error,
                             boost::ref(conn), boost::ref(t),
                             boost::ref(counter), dev));
  } else {
    mgmt::finalize();
  }
  mgmt::run();
}