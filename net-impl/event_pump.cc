#include "net/event_pump.h"
#include "net/epoll_reactor.h"
#include "net/fd_watcher.h"
#include "util/logging.h"
#include "util/mutex_lock.h"

namespace dlock {

EventPump::EventPump()
    : mutex_(),
      cond_(&mutex_),
      reactor_(new EpollReactor()),
      pending_change_(false),
      stop_(false) {
  StartThread();
}

EventPump::~EventPump() {
  stop_ = true;
  StopThread();
}

EventPump *GetInstance() {
  static EventPump pump;
  return &pump;
}

void EventPump::AddFdWatcher(int fd, EventType event, FdWatcher *watcher) {
  MutexLock lock(&mutex_);
  auto [it, ok] = fd_watchers_.insert({fd, nullptr});
  CHECK(ok || it->second == watcher);
  reactor_->WatchFd(fd, event);
  it->second = watcher;
}

void EventPump::DelFdWatcher(int fd, EventType event) {
  MutexLock lock(&mutex_);
  if (reactor_->UnWatchFd(fd, event)) {
    fd_watchers_.erase(fd);
  }
}

bool EventPump::HasFdWatcher(int fd, EventType event, FdWatcher *watcher) {
  MutexLock lock(&mutex_);
  auto it = fd_watchers_.find(fd);
  if (it == fd_watchers_.end() || it->second != watcher) {
    return false;
  }
  return reactor_->IsWatched(fd, event);
}

void EventPump::BlockRemoveFd(int fd) {
  MutexLock lock(&mutex_);
  reactor_->UnWatchFd(fd, RDWR);
  pending_change_ = true;
  cond_.Wait();
  fd_watchers_.erase(fd);
}

void EventPump::ThreadEntry() {
  std::vector<int> readable;
  std::vector<int> writable;
  while (!stop_) {
    {
      MutexLock lock(&mutex_);
      if (pending_change_) {
        pending_change_ = false;
        cond_.Signal();
      }
    }
    readable.clear();
    writable.clear();
    reactor_->WaitReady(&readable, &writable);

    if (readable.empty() && writable.empty()) {
      continue;
    }

    for (auto fd : readable) {
      MutexLock lock(&mutex_);
      auto it = fd_watchers_.find(fd);
      if (it != fd_watchers_.end()) {
        it->second->OnReadable(fd);
      }
    }
    for (auto fd : writable) {
      MutexLock lock(&mutex_);
      auto it = fd_watchers_.find(fd);
      if (it != fd_watchers_.end()) {
        it->second->OnWritable(fd);
      }
    }
  }
}

}  // namespace dlock