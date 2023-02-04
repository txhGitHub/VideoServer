#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
typedef u_int32_t ipv4AddressBits;
typedef u_int8_t ipv6AddressBits[16]; // 128 bits

#ifndef _GROUPSOCK_UTILS_HH
#define _GROUPSOCK_UTILS_HH
class GroupSocketUtils
{
private:
  /* data */
public:
  GroupSocketUtils(/* args */);
  ~GroupSocketUtils();

  static bool isBadIPv4AddressForUs(ipv4AddressBits addr);
  static bool isBadIPv6AddressForUs(ipv6AddressBits addr);
  static bool isBadAddressForUs(struct sockaddr const& addr);
  static struct sockaddr_storage const& nullAddress(int addressFamily, struct sockaddr_storage& nullIPAddress);
  static bool addressIsNull(sockaddr_storage const& address);
  static void copyAddress(struct sockaddr_storage& to, struct sockaddr const* from);
};
#endif