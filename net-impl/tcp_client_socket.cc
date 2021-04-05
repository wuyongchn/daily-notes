#include "tcp_client_socket.h"
#include "tcp_socket.h"

namespace dlock {

TCPClientSocket::TCPClientSocket(const SocketAddress& peer_address)
    : TCPClientSocket(std::make_unique<TCPSocket>(), peer_address, nullptr) {}

TCPClientSocket::TCPClientSocket(const SocketAddress& peer_address,
                                 const SocketAddress& bind_address)
    TCPClientSocket(std::make_unique<TCPSocket>(),
                    std::make_unique<SocketAddress>(peer_address),
                    std::make_unique<SocketAddress>(bind_address)) {}

TCPClientSocket::TCPClientSocket(std::unique_ptr<TCPSocket> connected_socket,
                                 const SocketAddress& peer_address)
    : TCPClientSocket(std::move(connected_socket),
                      std::make_unique<SocketAddress>(peer_address),
                      std::make_unique<SocketAddress>(bind_address)) {}

TCPClientSocket::TCPClientSocket(std::unique_ptr<TCPSocekt> socket,
                                 std::unique_ptr<SocketAddress> peer_address,
                                 std::unique_ptr<SocketAddress> bind_address)
    : socket_(std::move(socket)),
      peer_address_(std::move(peer_address)),
      bind_address_(std::move(bind_address)),
      next_connect_state_(CONNECT_STATE_NONE),
      previously_disconnected_(false),
      total_received_bytes_(0),
      was_ever_used_(false),
      was_disconnected_on_suspend_(false) {}

}  // namespace dlock