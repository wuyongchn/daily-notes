#ifndef DLOCK_NET_TCP_SOCKET_H_
#define DLOCK_NET_TCP_SOCKET_H_

#include "net/fd_watcher.h"
#include "scoped_refptr.h"

namespace dlock {

class IOBuffer;
class SocketAddress;

const int kInvalidSocket = -1;

class TCPSocket : public FdWatcher {
 public:
  TCPSocket();
  ~TCPSocket() override;

  int Open();
  int AdoptConnectedSocket(int socket_fd, const SocketAddress& peer_address);
  int AdoptUnconnectedSocket(int socket_fd);

  int ReleaseConnectedSocket();

  int Bind(const SocketAddress& address);

  int Listen(int backlog);
  int Accept(std::unique_ptr<TCPSocket>* socket);

  int Connect(const SocketAddress& address);

  bool IsConnected() const;
  bool IsConnectedAndIdle() const;

  int Read(IOBuffer* buf, int buf_len);
  int Write(IOBuffer* buf, int buf_len);

  int GetLocalAddress(SocketAddress* address) const;
  int GetPeerAddress(SocketAddress* address) const;
  void SetPeerAddress(const SocketAddress& address);
  // Returns true if peer address has been set regardless of socket state.
  bool HasPeerAddress() const;

  int AllowAddressReuse();
  int SetReveiveBufferSize(int32_t size);
  int SetSendBufferSize(int32_t size);
  bool SetKeepAlive(bool eable, int delay);
  bool SetNoDelay(bool no_dealy);

  void Close();
  int socket_fd() const { return socket_fd_; }

 private:
  void OnReadable(int fd) override;
  void OnWritable(int fd) override;

  int socket_fd_;
  scoped_refptr<IOBuffer> read_buf_;
  int read_buf_len_;
  scoped_refptr<IOBuffer> write_buf_;
  int write_buf_len_;
  bool waiting_connect_;
  std::unique_ptr<SocketAddress> peer_address_;

  DISALLOW_COPY_AND_ASSIGN(TCPSocket);
};

}  // namespace dlock

#endif