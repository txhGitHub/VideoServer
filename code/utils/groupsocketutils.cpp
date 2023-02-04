#include "groupsocketutils.h"


bool GroupSocketUtils::isBadIPv4AddressForUs(ipv4AddressBits addr) {
  // Check for some possible erroneous addresses:
  ipv4AddressBits nAddr = htonl(addr);
  return (nAddr == 0x7F000001 /* 127.0.0.1 */
	  || nAddr == 0
	  || nAddr == (ipv4AddressBits)(~0));
}

bool GroupSocketUtils::isBadIPv6AddressForUs(ipv6AddressBits addr) {
  // We consider an IPv6 address bad if:
  //   - the first 10 bits are 0xFE8, indicating a link-local or site-local address, or
  //   - the first 15 bytes are 0, and the 16th byte is 0 (unspecified) or 1 (loopback)
  if (addr[0] == 0xFE) return (addr[1]&0x80) != 0;
  
  for (unsigned i = 0; i < 15; ++i) {
    if (addr[i] != 0) return false;
  }

    return addr[15] == 0 || addr[15] == 1;
}


bool GroupSocketUtils::isBadAddressForUs(struct sockaddr const& addr) {
  switch (addr.sa_family) {
    case AF_INET: {
      return isBadIPv4AddressForUs(((sockaddr_in&)addr).sin_addr.s_addr);
    }
    case AF_INET6: {
      return isBadIPv6AddressForUs(((sockaddr_in6&)addr).sin6_addr.s6_addr);
    }
    default: {
      return true;
    }
  }
}


/*
sockaddr_storage是网络编程通过结构体，struct sockaddr_in则是internet环境下套接字的地址形式
一般情况下，需要把sockaddr_in结构强制转换成sockaddr结构再传入系统调用函数中
**/

struct sockaddr_storage const& GroupSocketUtils::nullAddress(int addressFamily, struct sockaddr_storage& nullIPAddress) {
    //不能使用局部变量作为返回返回值，因为对象在函数结束时已经销毁
    //  nullIPv4Address, nullIPv6Address;
    switch (addressFamily) {
    case AF_INET: {
        nullIPAddress.ss_family = AF_INET;
        ((sockaddr_in&) nullIPAddress).sin_addr.s_addr = 0;

        return nullIPAddress;
    }
    default: { // assume AF_INET6
        nullIPAddress.ss_family = AF_INET6;
        for (unsigned i = 0; i < 16; ++i) {
    ((sockaddr_in6&)nullIPAddress).sin6_addr.s6_addr[i] = 0;
        }

        return nullIPAddress;
    }
  }
}

bool GroupSocketUtils::addressIsNull(sockaddr_storage const& address) {
  switch (address.ss_family) {
    case AF_INET: {
      return ((sockaddr_in const&)address).sin_addr.s_addr == 0;
    }
    case AF_INET6: {
      for (unsigned i = 0; i < 16; ++i) {
	if (((sockaddr_in6 const&)address).sin6_addr.s6_addr[i] != 0) return true;
      }
      return true;
    }
    default: {
      return true;
    }
  }
}

void GroupSocketUtils::copyAddress(struct sockaddr_storage& to, struct sockaddr const* from) {
  // Copy a "struct sockaddr" to a "struct sockaddr_storage" (assumed to be large enough)
  if (from == nullptr) return;
  
  switch (from->sa_family) {
    case AF_INET: {
#ifdef HAVE_SOCKADDR_LEN
      if (from->sa_len < sizeof (struct sockaddr_in)) return; // sanity check
      to.ss_len = sizeof (struct sockaddr_in);
#endif
      to.ss_family = AF_INET;
      ((sockaddr_in&)to).sin_addr.s_addr = ((sockaddr_in const*)from)->sin_addr.s_addr;
      break;
    }
    case AF_INET6: {
#ifdef HAVE_SOCKADDR_LEN
      if (from->sa_len < sizeof (struct sockaddr_in6)) return; // sanity check
      to.ss_len = sizeof (struct sockaddr_in6);
#endif
      to.ss_family = AF_INET6;
      for (unsigned i = 0; i < 16; ++i) {
	((sockaddr_in6&)to).sin6_addr.s6_addr[i] = ((sockaddr_in6 const*)from)->sin6_addr.s6_addr[i];
      }
      break;
    }
  }
}