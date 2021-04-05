#ifndef DLOCK_NET_FD_WATCHER_H_
#define DLOCK_NET_FD_WATCHER_H_

namespace dlock {

class FdWatcher {
 public:
  virtual void OnReadable(int fd) = 0;
  virtual void OnWritable(int fd) = 0;
};

}  // namespace dlock

#endif