/**
 * @file client_request.h
 * @author Pengcheng Xu <jsteward@pku.edu.cn>
 * @brief Request from client.
 * @version 0.1
 * @date 2019-10-07
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef __KHTCPC_CLIENT_REQUEST_H_
#define __KHTCPC_CLIENT_REQUEST_H_

#include <netinet/in.h>
#include <netpacket/packet.h>

#define IFNAMSIZE 16

namespace khtcpc {

enum request_type {
  FIND_DEVICE,
  GET_DEVICE_MAC,
  GET_DEVICE_IP,

  ETHERNET_READ,
  ETHERNET_WRITE,

  ARP_READ,
  ARP_WRITE,
};

struct request {
  request_type type;
  int id;
  int payload_len;
  union {
    struct {
      char name[IFNAMSIZE];
    } find_device;
    struct {
      int dev_id;
    } get_device_mac;
    struct {
      int dev_id;
    } get_device_ip;

    struct {
      int dev_id;
    } eth_read;
    struct {
      int dev_id;
      uint16_t ethertype;
      struct sockaddr_ll mac;
    } eth_write;

    struct {
      int dev_id;
    } arp_read;
    struct {
      int dev_id;
      int opcode;
      struct sockaddr_ll sender_mac;
      struct sockaddr_in sender_ip;
      struct sockaddr_ll target_mac;
      struct sockaddr_in target_ip;
    } arp_write;
  };
};

struct response {
  request_type type;
  int id;
  int payload_len;
  union {
    struct {
      int dev_id;
    } find_device;
    struct {
      struct sockaddr_ll mac;
    } get_device_mac;
    struct {
      int count;
    } get_device_ip;

    struct {
      int dev_id;
      uint16_t ethertype;
    } eth_read;
    struct {
      int dev_id;
    } eth_write;

    struct {
      int dev_id;
      int opcode;
      struct sockaddr_ll sender_mac;
      struct sockaddr_in sender_ip;
      struct sockaddr_ll target_mac;
      struct sockaddr_in target_ip;
    } arp_read;
    struct {
      int dev_id;
    } arp_write;
  };
};

} // namespace khtcpc

#endif