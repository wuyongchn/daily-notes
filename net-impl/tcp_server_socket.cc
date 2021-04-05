#include "net/tcp_server_socket.h"
#include "net/socket_address.h"
#include "net/tcp_socket.h"

namespace dlock {

TCPServerSocket::TCPServerSocket()
    : TCPServerSocket(std::make_unique<TCPSocket>()) {}

TCPServerSocket::TCPServerSocket(std::unique_ptr<TCPSocket> socket)
    : socket_(std::move(socket)), pending_accept_(false) {}

TCPServerSocket::~TCPServerSocket() = default;

int TCPServerSocket::AdoptSocket(int socket_fd) {
  return socket_->AdoptUnconnectedSocket(socket);
}

int TCPServerSocket::Listen(const SocketAddress& address, int backlog) {
  int ret = socket_->Open();
  if (ret) {
    return ret;
  }

  ret = socket_->SetDefaultOptionsForServer();
  if (ret) {
    socket_->Close();
    return ret;
  }

  ret = socket_->Bind(address);
  if (ret) {
    socket_->Close();
    return ret;
  }

  ret = socket_->Listen(backlog);
  if (ret) {
    socket_->Close();
    return ret;
  }
  return 0;
}

int TCPServerSocket::ListenWithAddressAndPort(const std::string& ip,
                                              uint16_t port, int backlog) {
  SocketAddress address(ip, port);
  return Listen(address, backlog);
}

int TCPServerSocket::GetLocalAddress(SocketAddress* address) const {
  return socket_->GetLocalAddress(address);
}

int TCPServerSocket::Accept(std::unique_ptr<TCPConnection>* connection,
                            SocketAddress* peer_address) {
  int ret = socket_->Accept(&accepted_socket_, &accepted_address_);
  if (!ret) {
    ret = ConvertAcccptedSocket(connection, peer_address);
  }
  return ret;
}

}  // namespace dlock