#ifndef DLOCK_NET_EVENT_TYPE_H_
#define DLOCK_NET_EVENT_TYPE_H_

namespace dlock {

enum EventType {
  READ = 0x1,
  WRITE = 0x10,
  RDWR = 0x11,
  MASK = ~0x11,
};
}

#endif