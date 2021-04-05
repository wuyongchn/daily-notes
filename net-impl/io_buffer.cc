#include "net/io_buffer.h"
#include <string.h>

namespace dlock {

IOBuffer::IOBuffer() : data_(nullptr) {}

IOBuffer::IOBuffer(size_t buffer_size) {
  CHECK_GE(buffer_size, 0);
  data_ = new char[buffer_size];
}

IOBuffer::IOBuffer(char* data) : data_(data) {}

IOBuffer::~IOBuffer() {
  delete[] data_;
  data_ = nullptr;
}

IOBufferWithSize::IOBufferWithSize(size_t size) : IOBuffer(size), size_(size) {}

IOBufferWithSize::IOBufferWithSize(char* data, size_t size)
    : IOBuffer(data), size_(size) {
  CHECK_GE(size, 0);
}

IOBufferWithSize::~IOBufferWithSize() = default;

StringIOBuffer::StringIOBuffer(const std::string& s)
    : IOBuffer(nullptr), string_data_(s) {
  data_ = const_cast<char*>(string_data_.data());
}

StringIOBuffer::StringIOBuffer(std::unique_ptr<std::string> s)
    : IOBuffer(nullptr) {
  string_data_.swap(*s.get());
  data_ = const_cast<char*>(string_data_.data());
}

StringIOBuffer::~StringIOBuffer() { data_ = nullptr; }

DrainableIOBuffer::DrainableIOBuffer(scoped_refptr<IOBuffer> base, int size)
    : IOBuffer(base->data()), base_(std::move(base)), size_(size), used_(0) {}

DrainableIOBuffer::DrainableIOBuffer(scoped_refptr<IOBuffer> base, size_t size)
    : IOBuffer(base->data()), base_(std::move(base)), size_(size), used_(0) {}

void DrainableIOBuffer::DidConsume(int bytes) { SetOffset(used_ + bytes); }

int DrainableIOBuffer::BytesRemaining() const { return size_ - used_; }

int DrainableIOBuffer::BytesConsumed() const { return used_; }

DrainableIOBuffer::~DrainableIOBuffer() {
  // The buffer is owned by the |base_| instance.
  data_ = nullptr;
}

GrowableIOBuffer::GrowableIOBuffer() : IOBuffer(), capacity_(0), offset_(0) {}

void GrowableIOBuffer::SetCapacity(int capacity) {
  CHECK_GE(capacity, 0);
  std::unique_ptr<char[]> buf(new char[capacity]);
  if (buf) {
    int size = std::min(offset_, capacity);
    memmove(buf.get(), real_data_.get(), size);
    real_data_.swap(buf);
  }
  capacity_ = capacity;
  if (offset_ > capacity)
    set_offset(capacity);
  else
    set_offset(offset_);
}

void GrowableIOBuffer::set_offset(int offset) {
  CHECK_GE(offset, 0);
  CHECK_GE_LE(offset, capacity_);
  offset_ = offset;
  data_ = real_data_.get() + offset;
}

int GrowableIOBuffer::RemainingCapacity() { return capacity_ - offset_; }

char* GrowableIOBuffer::StartOfBuffer() { return real_data_.get(); }

GrowableIOBuffer::~GrowableIOBuffer() { data_ = nullptr; }

}  // namespace dlock