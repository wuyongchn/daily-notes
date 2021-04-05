#ifndef DLOCK_NET_EPOLL_REACTOR_H_
#define DLOCK_NET_EPOLL_REACTOR_H_

#include <sys/epoll.h>
#include <unordered_map>
#include <vector>
#include "net/event_type.h"

namespace dlock {

class EpollReactor {
 public:
  EpollReactor();
  ~EpollReactor();

  void WatchFd(int fd, EventType event);
  bool UnWatchFd(int fd, EventType event);
  bool IsWatched(int fd, EventType event);
  void WaitReady(std::vector<int> *readable, std::vector<int> *writable);

 private:
  static const int kEventInitNum;

  int epfd_;
  std::vector<struct epoll_event> ready_;
  std::unordered_map<int, unsigned char> fd_status_;
};
}  // namespace dlock

#endif