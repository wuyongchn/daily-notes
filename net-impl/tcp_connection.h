#ifndef DLOCK_NET_TCP_CONNECTION_H_
#define DLOCK_NET_TCP_CONNECTION_H_

namespace dlock {

class IOBuffer;
class SocketAddress;

class TCPConnection {
 public:
  TCPConnection() = default;
  virtual ~TCPConnection() = default;
  virtual int Read(IOBuffer* buf, int buf_len) = 0;
  virtual int Write(IOBuffer* buf, int buf_len) = 0;
  virtual int SetReceiveBufferSize(int32_t size) = 0;
  virtual int SetSendBufferSize(int32_t size) = 0;
  virtual int Connect() = 0;
  virtual void Disconnect() = 0;
  virtual bool IsConnected() const = 0;
  virtual bool IsConnectedAndIdle() const = 0;
  virtual int GetPeerAddress(SocketAddress* address) const = 0;
  virtual int GetLocalAddress(SocketAddress* address) const = 0;
  virtual bool WasEverUsed() const = 0;
  virtual int64_t GetTotalReceivedBytes() const = 0;
};

}  // namespace dlock

#endif