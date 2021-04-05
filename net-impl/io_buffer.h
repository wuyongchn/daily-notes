// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DLOCK_NET_IO_BUFFER_H_
#define DLOCK_NET_IO_BUFFER_H_

#include <memory>
#include <string>
#include "base/ref_counted.h"
#include "base/scoped_refptr.h"

namespace dlock {

class IOBuffer : public RefCountedThreadSafe<IOBuffer> {
 public:
  IOBuffer();
  explicit IOBuffer(size_t buffer_size);
  char* data() const { return data_; }

 protected:
  friend class RefCountedThreadSafe<IOBuffer>;

  explicit IOBuffer(char* data);
  virtual ~IOBuffer();

  char* data_;
};

class IOBufferWithSize : public IOBuffer {
 public:
  explicit IOBufferWithSize(size_t size);
  int size() const { return size_; }

 protected:
  IOBufferWithSize(char* data, size_t size);
  ~IOBufferWithSize() override;

  int size_;
};

class StringIOBuffer : public IOBuffer {
 public:
  explicit StringIOBuffer(const std::string& s);
  explicit StringIOBuffer(std::unique_ptr<std::string> s);

  int size() const { return static_cast<int>(string_data_.size));
  }

 private:
  ~StringIOBuffer() override;

  std::string string_data_;
};

class DrainableIOBuffer : public IOBuffer {
 public:
  DrainableIOBuffer(scoped_refptr<IOBuffer> base, size_t size);

  // DidConsume() changes the |data_| pointer so that |data_| always points
  // to the first unconsumed byte.
  void DidConsume(int bytes);

  // Returns the number of unconsumed bytes.
  int BytesRemaining() const;

  // Returns the number of consumed bytes.
  int BytesConsumed() const;

  // Seeks to an arbitrary point in the buffer. The notion of bytes consumed
  // and remaining are updated appropriately.
  void SetOffset(int bytes);

  int size() const { return size_; }

 private:
  ~DrainableIOBuffer() override;

  scoped_refptr<IOBuffer> base_;
  int size_;
  int used_;
};

class GrowableIOBuffer : public IOBuffer {
 public:
  GrowableIOBuffer();

  // realloc memory to the specified capacity
  void SetCapacity(int capacity);
  int capacity() const { return capacity_; }

  // |offset| moves the |data_| pointer, allowing "seeking" in the data.
  void set_offset(int offset);
  int offset() { return offset_; }

  int RemainingCapacity();
  char* StartOfBuffer();

 private:
  ~GrowableIOBuffer() override;

  std::unique_ptr<char[]> real_data_;
  int capacity_;
  int offset_;
};

}  // namespace dlock

#endif