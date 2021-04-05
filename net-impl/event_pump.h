#ifndef DLOCK_NET_EVENT_PUMP_H_
#define DLOCK_NET_EVENT_PUMP_H_

#include <memory>
#include <unordered_map>
#include "base/noncopyable.h"
#include "base/sync.h"
#include "base/thread.h"
#include "net/event_type.h"

namespace dlock {

class EpollReactor;
class FdWatcher;

class EventPump : public Thread {
 public:
  EventPump();
  ~EventPump();
  static EventPump *GetInstance();
  void AddFdWatcher(int fd, EventType event, FdWatcher *watcher);
  void DelFdWatcher(int fd, EventType event);
  bool HasFdWatcher(int fd, EventType event, FdWatcher *watcher);
  void BlockRemoveFd(int fd);

 private:
  void ThreadEntry() override;

  Mutex mutex_;
  CondVar cond_;
  std::unique_ptr<EpollReactor> reactor_;
  bool pending_change_;
  bool stop_;
  std::unordered_map<int, FdWatcher *> fd_watchers_;

  DISALLOW_COPY_AND_ASSIGN(EventPump)
};

}  // namespace dlock

#endif