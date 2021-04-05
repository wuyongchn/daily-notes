#include "net/epoll_reactor.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "util/logging.h"

namespace dlock {

const int EpollReactor::kEventInitNum = 32;

EpollReactor::EpollReactor() : epfd_(::epoll_create(1)), ready_(kEventInitNum) {
  CHECK_NE(-1, epfd_);
}

EpollReactor::~EpollReactor() { close(epfd_); }

void EpollReactor::WatchFd(int fd, EventType event) {
  struct epoll_event ev;
  int op;
  auto [it, ok] = fd_status_.insert({fd, 0});
  if (ok) {
    // insert succeed, new fd;
    op = EPOLL_CTL_ADD;
  } else {
    op = EPOLL_CTL_MOD;
  }
  it->second |= static_cast<unsigned char>(event);

  ev.events = EPOLLET;
  ev.data.fd = fd;

  if (it->second & READ) {
    ev.events |= EPOLLIN;
  }
  if (it->second & WRITE) {
    ev.events |= EPOLLOUT;
  }

  LOG_ASSERT(epoll_ctl(epfd_, op, fd, &ev) == 0);
}

bool EpollReactor::UnWatchFd(int fd, EventType event) {
  auto it = fd_status_.find(fd);
  if (it == fd_status_.end()) {
    return false;
  }

  it->second &= ~static_cast<unsigned char>(event);
  int op;
  if (it->second) {
    op = EPOLL_CTL_MOD;
  } else {
    op = EPOLL_CTL_DEL;
  }

  struct epoll_event ev;
  ev.events = EPOLLET;
  ev.data.fd = fd;

  if (op == EPOLL_CTL_MOD) {
    if (it->second & READ) {
      ev.events |= EPOLLIN;
    }
    if (it->second & WRITE) {
      ev.events |= EPOLLOUT;
    }
  } else {
    fd_status_.erase(fd);
  }

  LOG_ASSERT(epoll_ctl(epfd_, op, fd, &ev) == 0);
  return op == EPOLL_CTL_DEL;
}

bool EpollReactor::IsWatched(int fd, EventType event) {
  auto it = fd_status_.find(fd);
  if (it == fd_status_.end()) {
    return false;
  }
  return it->second & MASK == event;
}

void EpollReactor::WaitReady(std::vector<int>* readable,
                             std::vector<int>* writable) {
  int nfds = epoll_wait(epfd_, ready_.data(), kEventInitNum, -1);
  for (int i = 0; i < nfds; ++i) {
    if (readable && ready_[i].events & EPOLLIN) {
      readable->push_back(ready_[i].data.fd);
    }
    if (writable && ready_[i].events & EPOLLOUT) {
      writable->push_back(ready_[i].data.fd);
    }
  }
  if (nfds == ready_.size()) {
    ready_.resize(nfds * 2);
  }
}

}  // namespace dlock