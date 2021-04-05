#ifndef DLOCK_NET_SOCKET_ADDRESS_H_
#define DLOCK_NET_SOCKET_ADDRESS_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <string>

namespace dlock {

class SockaddrHolder;

class SocketAddress {
 public:
  SocketAddress();
  SocketAddress(const std::string& ip, uint16_t port);

  bool FromString(const std::string& address /*ip:port*/);
  void FromSockaddrHolder(const SockaddrHolder& address);
  SockaddrHolder ToSockaddrHolder() const;
  std::string ToString() const;
  std::string ToStringWithoutPort() const;
  bool empty() const { return 0 == ip_ && 0 == port_; }
  uint32_t ip() const { return ip_; }
  uint16_t port() const { return port_; }

 private:
  friend bool operator<(const SocketAddress& lhs, const SocketAddress& rhs);
  friend bool operator>(const SocketAddress& lhs, const SocketAddress& rhs);
  friend bool operator==(const SocketAddress& lhs, const SocketAddress& rhs);
  friend bool operator!=(const SocketAddress& lhs, const SocketAddress& rhs);

  uint32_t ip_;    // network order
  uint16_t port_;  // network order
};

struct SockaddrHolder {
  SockaddrHolder();
  SockaddrHolder(const SockaddrHolder& other);
  SockaddrHolder& operator=(const SockaddrHolder& other);
  SocketAddress ToSocketAddress() const;

  struct sockaddr sock_addr;
  socklen_t addr_len;
  struct sockaddr* const addr;
};

inline bool operator<(const SocketAddress& lhs, const SocketAddress& rhs) {
  if (lhs.ip_ < rhs.ip_) {
    return true;
  }
  return lhs.ip_ == rhs.ip_ && lhs.port_ < rhs.port_;
}

inline bool operator>(const SocketAddress& lhs, const SocketAddress& rhs) {
  return rhs < lhs;
}

inline bool operator==(const SocketAddress& lhs, const SocketAddress& rhs) {
  return lhs.ip_ == rhs.ip_ && lhs.port_ == rhs.port_;
}
inline bool operator!=(const SocketAddress& lhs, const SocketAddress& rhs) {
  return !operator==(lhs, rhs);
}

}  // namespace dlock

#endif