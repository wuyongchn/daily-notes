#ifndef DLOCK_NET_TCP_SERVER_SOCKET_H_
#define DLOCK_NET_TCP_SERVER_SOCKET_H_

#include <memory>
#include "base/noncopyable.h"

namespace dlock {

class SocketAddress;
class TCPSocket;
class TCPConnection;

class TCPServerSocket {
 public:
  TCPServerSocket();
  explicit TCPServerSocket(std::unique_ptr<TCPSocket> socket);
  ~TCPServerSocket();
  int AdoptSocket(int socket_fd);
  int Listen(const SocketAddress& address, int backlog);
  int ListenWithAddressAndPort(const std::string& address, uint16_t port,
                               int backlog);
  int GetLocalAddress(SocketAddress* address) const;
  int Accept(std::unique_ptr<TCPConnection>* connection,
             SocketAddress* peer_address=nullptr);

 private:
  int ConvertAcccptedSocket(
      std::unique_ptr<TCPConnection>* output_accepted_connection,
      SocketAddress* output_accepted_address);

  std::unique_ptr<TCPSocket> socekt_;
  std::unique_ptr<TCPSocekt> accepted_socket_;
  SocketAddress accepted_address_;
  bool pending_accept_;

  DISALLOW_COPY_AND_ASSIGN(TCPServerSocket);
};

}  // namespace dlock

#endif