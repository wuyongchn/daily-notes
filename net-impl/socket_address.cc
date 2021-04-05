#include "net/socket_address.h"
#include <arpa/inet.h>
#include <string.h>
#include "util/logging.h"

namespace dlock {

static bool ParseIp(const std::string& address, uint32_t* ip) {
  if (address.empty() || nullptr == ip) {
    return false;
  }
  const char* str = address.c_str();
  while (isspace(*str)) ++str;
  if (inet_pton(AF_INET, str, ip)) {
    return true;
  }
  return false;
}

SocketAddress::SocketAddress() : ip_(0), port_(0) {}

SocketAddress::SocketAddress(const std::string& address, uint16_t port)
    : ip_(0), port_(htons(port)) {
  LOG_ASSERT(ParseIp(address, &ip_));
}

bool SocketAddress::FromString(const std::string& address) {
  if (address.empty()) {
    return false;
  }
  auto pos = address.find_first_of(':');
  if (pos == std::string::npos) {
    return false;
  }
  std::string ip_str = address.substr(0, pos);
  std::string port_str = address.substr(pos + 1);

  if (!ParseIp(ip_str, &ip_)) {
    return false;
  }

  char* end = nullptr;
  int port = strtol(port_str.c_str(), &end, 10);
  if (end == port_str.c_str()) {
    return false;
  } else if (*end) {
    for (++end; isspace(*end); ++end)
      ;
    if (*end) {
      return false;
    }
  }
  if (port < 0 || port > 65535) {
    return false;
  }
  port_ = htons(port);
  return true;
}

void SocketAddress::FromSockaddrHolder(const SockaddrHolder& sock_addr) {
  auto ptr = reinterpret_cast<struct sockaddr_in*>(sock_addr.addr);
  port_ = ptr->sin_port;
  ip_ = (ptr->sin_addr).s_addr;
}

SockaddrHolder SocketAddress::ToSockaddrHolder() const {
  SockaddrHolder ret;
  auto ptr = reinterpret_cast<struct sockaddr_in*>(ret.addr);
  ptr->sin_family = AF_INET;
  ptr->sin_port = port_;
  (ptr->sin_addr).s_addr = ip_;
  ret.addr_len = sizeof(ret.sock_addr);
  return ret;
}

std::string SocketAddress::ToString() const {
  char buf[INET_ADDRSTRLEN + 16] = {0};
  if (inet_ntop(AF_INET, &ip_, buf, INET6_ADDRSTRLEN) == nullptr) {
    return {};
  }
  char* ptr = buf + strlen(buf);
  *ptr++ = ':';
  snprintf(ptr, 16, "%d", ntohs(port_));

  return std::string(buf);
}

std::string SocketAddress::ToStringWithoutPort() const {
  char buf[INET_ADDRSTRLEN + 16] = {0};
  if (inet_ntop(AF_INET, &ip_, buf, INET6_ADDRSTRLEN) == nullptr) {
    return {};
  }
  return std::string(buf);
}

bool operator<(const SocketAddress& lhs, const SocketAddress& rhs);
bool operator==(const SocketAddress& lhs, const SocketAddress& rhs);
bool operator!=(const SocketAddress& lhs, const SocketAddress& rhs);

SockaddrHolder::SockaddrHolder()
    : addr_len(sizeof(sock_addr)),
      addr(reinterpret_cast<struct sockaddr*>(&sock_addr)) {
  memset(addr, 0, addr_len);
}

SockaddrHolder::SockaddrHolder(const SockaddrHolder& other)
    : addr_len(sizeof(sock_addr)),
      addr(reinterpret_cast<struct sockaddr*>(&sock_addr)) {
  memcpy(addr, other.addr, addr_len);
}

SockaddrHolder& SockaddrHolder::operator=(const SockaddrHolder& other) {
  addr_len = other.addr_len;
  memcpy(addr, other.addr, addr_len);
}

SocketAddress SockaddrHolder::ToSocketAddress() const {
  SocketAddress address;
  address.FromSockaddrHolder(*this);
  return address;
}

}  // namespace dlock