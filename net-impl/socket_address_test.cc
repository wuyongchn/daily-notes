#include "net/socket_address.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include "util/logging.h"
#include "util/unittest.h"

namespace dlock {
namespace unittest {

struct TestData {
  std::string ip;
  uint16_t port;
} valid_tests[] = {{"0.0.0.0", 2343},        {"121.45.33.1", 8953},
                   {"127.0.0.1", 3243},      {"173.45.0.1", 8945},
                   {"192.168.1.1", 3252},    {"239.255.255.254", 7239},
                   {"255.255.255.255", 2555}};

std::string invalid_tests[] = {
    "-121.45.33.1:4567",     "123.-2.3.5:34334",     "127.0.0.-1:3233",
    "173.-45.0.1:4232",      "301.1.1.1:3343",       "192.168.1.1:-1",
    "239.255.255.254:65536", "255-255-255-255:2353", "192:145:1:1:1343"};

UNITTEST_DEFINITION(SocketAddressTest);

TEST(SocketAddressTest, TestConstructor) {
  {
    SocketAddress address;
    CHECK_EQ(address.ip(), 0);
    CHECK_EQ(address.port(), 0);
  }
  for (const auto& test : valid_tests) {
    SocketAddress address(test.ip, test.port);
    CHECK_EQ(address.ip(), inet_addr(test.ip.c_str()));
    CHECK_EQ(address.port(), htons(test.port));
  }
}

TEST(SocketAddressTest, TestAssignment) {
  uint16_t port = 0;
  for (const auto& test : valid_tests) {
    SocketAddress src(test.ip, ++port);
    SocketAddress dst = src;
    CHECK_EQ(src.port(), dst.port());
    CHECK_EQ(src.ip(), dst.ip());
  }
}

TEST(SocketAddressTest, TestCopy) {
  uint16_t port = 0;
  for (const auto& test : valid_tests) {
    SocketAddress src(test.ip, ++port);
    SocketAddress dst(src);
    CHECK_EQ(src.port(), dst.port());
    CHECK_EQ(dst.ip(), dst.ip());
  }
}

TEST(SocketAddressTest, TestToString) {
  for (const auto& test : valid_tests) {
    SocketAddress address(test.ip, test.port);
    std::string ip_port = test.ip + ":" + std::to_string(test.port);
    CHECK_EQ(address.ToStringWithoutPort(), test.ip);
    CHECK_EQ(address.ToString(), ip_port);
  }
}

TEST(SocketAddressTest, TestFromString) {
  for (const auto& test : valid_tests) {
    SocketAddress address(test.ip, test.port);
    std::string ip_port = test.ip + ":" + std::to_string(test.port);

    SocketAddress other(test.ip, test.port);
    CHECK_EQ(true, other.FromString(ip_port));
    CHECK_EQ(address.ip(), other.ip());
    CHECK_EQ(address.port(), other.port());
  }

  for (const auto& test : invalid_tests) {
    SocketAddress address;
    CHECK_EQ(false, address.FromString(test));
  }
}

TEST(SocketAddressTest, TestToSockaddrHolder) {
  for (const auto& test : valid_tests) {
    SocketAddress address(test.ip, test.port);
    SockaddrHolder holder = address.ToSockaddrHolder();
    auto ptr = reinterpret_cast<struct sockaddr_in*>(holder.addr);
    CHECK_EQ(address.ip(), ptr->sin_addr.s_addr);
    CHECK_EQ(address.port(), ptr->sin_port);
  }
}

TEST(SocketAddressTest, TestFromSockaddrHolder) {
  for (const auto& test : valid_tests) {
    SocketAddress address(test.ip, test.port);
    SockaddrHolder holder = address.ToSockaddrHolder();
    SocketAddress other;
    other.FromSockaddrHolder(holder);
    CHECK_EQ(address.ip(), other.ip());
    CHECK_EQ(address.port(), other.port());
  }
}

TEST(SocketAddressTest, TestComparisons) {
  {
    std::string ip_port1 = "192.168.1.1:2343";
    SocketAddress address, address2;
    LOG_ASSERT(address.FromString(ip_port1));
    LOG_ASSERT(address2.FromString(ip_port1));
    CHECK_EQ(address, address2);
    SocketAddress other = address;
    CHECK_EQ(address, other);
  }
  {
    std::string ip_port1 = "192.168.1.1:2343";
    std::string ip_port2 = "192.168.1.1:2344";
    SocketAddress address1, address2;
    LOG_ASSERT(address1.FromString(ip_port1));
    LOG_ASSERT(address2.FromString(ip_port2));
    CHECK_NE(address1, address2);
    CHECK_LT(address1, address2);
    CHECK_GT(address2, address1);
  }
  {
    std::string ip_port1 = "192.167.1.1:2343";
    std::string ip_port2 = "192.168.1.1:2343";
    SocketAddress address1, address2;
    LOG_ASSERT(address1.FromString(ip_port1));
    LOG_ASSERT(address2.FromString(ip_port2));
    CHECK_NE(address1, address2);
    CHECK_LT(address1, address2);
    CHECK_GT(address2, address1);
  }
  {
    std::string ip_port1 = "192.167.1.1:2343";
    std::string ip_port2 = "192.168.1.1:4546";
    SocketAddress address1, address2;
    LOG_ASSERT(address1.FromString(ip_port1));
    LOG_ASSERT(address2.FromString(ip_port2));
    CHECK_NE(address1, address2);
    CHECK_LT(address1, address2);
    CHECK_GT(address2, address1);
  }
}

}  // namespace unittest
}  // namespace dlock

UNITTEST_RUN(SocketAddressTest)