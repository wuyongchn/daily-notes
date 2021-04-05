#ifndef DLOCK_NET_TCP_CLIENT_SOCKET_H_
#define DLOCK_NET_TCP_CLIENT_SOCKET_H_

#include <memory>
#include "base/noncopyable.h"
#include "net/tcp_connection.h"

namespace dlock {

class SocketAddress;
class TCPSocekt;

class TCPClientSocket : public TCPConnection {
 public:
  explicit TCPClientSocket(const SocketAddress& peer_address);
  TCPClientSocket(const SocketAddress& peer_address,
                  const SocketAddress& bind_address);
  TCPClientSocket(std::unique_ptr<TCPSocket> connected_socket,
                  const SocketAddress& peer_address);
  ~TCPClientSocket() override;

  int Bind(const SocketAddress& local_addr);
  bool SetNoDelay(bool no_delay);
  bool SetKeepAlive(bool enable, int delay_secs);

  // TCP connection implementation
  vint Read(IOBuffer* buf, int buf_len) override;
  int Write(IOBuffer* buf, int buf_len) override;
  int SetReceiveBufferSize(int32_t size) override;
  int SetSendBufferSize(int32_t size) override;
  int Connect() override;
  void Disconnect() override;
  bool IsConnected() const override;
  bool IsConnectedAndIdle() constoverride;
  int GetPeerAddress(SocketAddress* address) const override;
  int GetLocalAddress(SocketAddress* address) const override;
  bool WasEverUsed() const override;
  int64_t GetTotalReceivedBytes() const override;

 private:
  TCPClientSocket(std::unique_ptr<TCPSocekt> socket,
                  std::unique_ptr<SocketAddress> peer_address,
                  std::unique_ptr<SocketAddress> bind_address);
  enum ConnectState {
    CONNECT_STATE_CONNECT,
    CONNECT_STATE_CONNECT_COMPLETE,
    CONNECT_STATE_NONE,
  };

  std::unique_ptr<TCPSocekt> socket_;
  std::unique_ptr<SocketAddress> peer_address_;
  std::unique_ptr<SocketAddress> bind_address_;

  ConnectState next_connect_state_;
  bool previously_disconnected_;
  int64_t total_received_bytes_;
  bool was_ever_used_;
  bool was_disconnected_on_suspend_;

  DISALLOW_COPY_AND_ASSIGN(TCPClientSocket);
};

}  // namespace dlock

#endif