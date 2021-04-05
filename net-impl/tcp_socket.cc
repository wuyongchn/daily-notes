#include "net/tcp_socket.h"
#include "util/logging.h"

namespace dlock {

TCPSocket::TCPSocekt()
    : socket_fd_(kInvalidSocket),
      read_buf_len_(0),
      write_buf_len_(0),
      waiting_connect_(false) {}

TCPSocekt::~TCPSocekt() { Close(); }

int TcpSocket::Open() {
  CHECK_EQ(kInvalidSocket, socket_fd_);
  socket_fd_ = ::socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (socket_fd_ < 0) {
    LOG_ERROR("::socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0) failed, %s",
              strerror(errno))
    return -1;
  }
  if (!SetNonBlocking(socket_fd_)) {
    Close();
    LOG_ERROR("SetNonBlocking() Failed")
    return -1;
  }
  return 0;
}

int TCPSocket::AdoptConnectedSocket(int socket_fd,
                                    const SocketAddress& peer_address) {
  int ret = AdoptUnconnectedSocket(socket_fd);
  SetPeerAddress(peer_address);
  return ret;
}

int TCPSocket::AdoptUnconnectedSocket(int socket_fd) {
  CHECK_EQ(kInvalidSocket, socket_fd_);
  socket_fd_ = socket_fd;

  if (!SetNonBlocking(socket_fd_)) {
    Close();
    LOG_ERROR("SetNonBlocking() failed. %s", strerror(errno));
    return -1;
  }
  return 0;
}

SocketDescriptor SocketPosix::ReleaseConnectedSocket() {
  // It's not safe to release a socket with a pending write.
  CHECK(!write_buf_);

  StopWatchingAndCleanUp(false /* close_socket */);
  SocketDescriptor socket_fd = socket_fd_;
  socket_fd_ = kInvalidSocket;
  return socket_fd;
}

int TCPSocket::Bind(const SocketAddress& address) {
  CHECK_NE(kInvalidSocket, socket_fd_);
  SockaddrHolder sock_addr = address.ToSockaddrHolder();
  if(::bind(socket_fd_, sock_addr.addr, sock_addr.len) {
    LOG_ERROR("Failed to call bind socket %d to the address %s, %s", sockfd,
              address.ToString().c_str(), strerror(errno));
    return -1;
  }
  return 0;
}

int TCPSocket::Listen(int backlog) {
  CHECK_NE(kInvalidSocket, socket_fd_);
  CHECK_LT(0, backlog);
  if (::listen(socket_fd_, backlog) == -1) {
    LOG_ERROR("Failed to call listen(%d, %d), %s", socket_fd_, backlog,
              strerror(errno));
    return -1;
  }
  return 0;
}

int TCPSocekt::Accept(std::unique_ptr<TCPSocket>* socket) {
  CHECK_NE(kInvalidSocket, socket_fd_);
  CHECK(socket);
  SockaddrHolder peer_addr;
  int new_socket = accept(socket_fd_, peer_address.addr, peer_addr.len);
  if (new_socket == -1) {
    LOG_ERROR("accept failed, %s", strerror(errno));
    return -1;
  }

  std::unique_ptr<TCPSocket> accepted_socket(new TCPSocket);
  accepted_socket->AdoptConnectedSocket(new_socket,
                                        peer_addr.ToSocketAddress());
  *socket = std::move(accepted_socket);
  return 0;
}

int TCPSocket::Connect(const SocketAddress& address) {
  CHECK_NE(kInvalidSocket, socket_fd_);
  SetPeerAddress(address);
  SockaddrHolder peer_address = address.ToSockaddrHolder();

  if (::connect(socket_fd_, peer_address.addr, peer_address.len) != 0) {
    LOG_ERROR("connect failed. %s", strerror(errno));
    return -1;
  }
  return 0;
}

bool TCPSocket::IsConnected() const {
  if (kInvalidSocket == socket_fd_) return false;

  // Checks if connection is alive.
  char c;
  int rv = recv(socket_fd_, &c, 1, MSG_PEEK);
  if (rv == 0) return false;
  if (rv == -1 && errno != EAGAIN && errno != EWOULDBLOCK) return false;

  return true;
}

bool TCPSocket::IsConnectedAndIdle() const {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (socket_fd_ == kInvalidSocket) return false;

  // Check if connection is alive and we haven't received any data
  // unexpectedly.
  char c;
  int rv = recv(socket_fd_, &c, 1, MSG_PEEK);
  if (rv >= 0) return false;
  if (errno != EAGAIN && errno != EWOULDBLOCK) return false;

  return true;
}

int TCPSocket::Read(IOBuffer* buf, int buf_len) {
  CHECK_NE(kInvalidSocket, socket_fd_);
  return read(socket_fd_, buf->data(), buf_len);
}

int TCPSocket::Write(IOBuffer* buf, int buf_len) {
  CHECK_NE(kInvalidSocket, socket_fd_);
  CHECK_LT(0, buf_len);

  return send(socket_fd_, buf->data(), buf_len, MSG_NOSIGNAL);
}

SocketAddress TCPSocket::GetLocalAddress() const {
  SockaddrHolder address;
  if (::getsockname(socket_fd_, address.addr, &address.len) < 0) {
    memset(address.addr, 0, address.len);
  }
  return address.ToSocketAddress();
}

int TCPSocket::GetLocalAddress(SocketAddress* address) const {
  CHECK(address);

  return getsockname(socket_fd_, address->addr, &address->addr_len)
}

int TCPSocket::GetPeerAddress(SocketAddress* address) const {
  CHECK(address);

  if (!HasPeerAddress()) return -1;

  *address = *peer_address_;
  return 0;
}

void TCPSocket::SetPeerAddress(const SocketAddress& address) {
  CHECK(!peer_address_);
  peer_address_.reset(new SocketAddress(address));
}

bool TCPSocket::HasPeerAddress() const { return peer_address_ != nullptr; }

int TCPSocketPosix::AllowAddressReuse() {
  if (socket_) return -1;

  return SetReuseAddr(socket_->socket_fd(), true);
}

bool TCPSocket::SetKeepAlive(bool enable, int delay) {
  if (!socket_) return false;

  return SetTCPKeepAlive(socket_->socket_fd(), enable, delay);
}

bool TCPSocket::SetNoDelay(bool no_delay) {
  if (!socket_) return false;
  return SetTCPNoDelay(socket_->socket_fd(), no_delay) == OK;
}

int TCPSocketPosix::SetReceiveBufferSize(int32_t size) {
  if (socket_) return -1;
  return SetSocketReceiveBufferSize(socket_->socket_fd(), size);
}

int TCPSocketPosix::SetSendBufferSize(int32_t size) {
  if (socket_) return -1;
  return SetSocketSendBufferSize(socket_->socket_fd(), size);
}

void TCPSocket::Close() {
  CHECK_NE(kInvalidSocket, socket_fd_);
  ::close(socket_fd_);
}

}  // namespace dlock