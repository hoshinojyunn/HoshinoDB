#pragma once
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <string>

#include "tools/slice.h"

namespace HoshinoDB {
constexpr static const std::size_t fileBufferSize = 65536;

class PosixReadableFile {
 public:
  PosixReadableFile(Slice path) : filename_(path.Data()) {
    // fd_ = ::open(path.Data(), oflag);
    if (fd_ = ::open(path.Data(), O_RDONLY); fd_ != -1) {
      isopened_ = true;
    }
  }
  ~PosixReadableFile() {
    if (isopened_) this->Close();
  }
  auto Valid() -> bool;
  auto Close() -> bool;
  auto Read(Slice* result, char* const scratch, int offset,
            std::size_t nbytes) -> int;
  int GetFD() { return fd_; }

 private:
  int fd_;
  bool isopened_;
  std::string filename_;
};

class PosixWritableFile {
 public:
  PosixWritableFile(Slice filename) : filename_(filename.Data()), pos_(0) {
    if (fd_ = ::open(filename.Data(), O_CREAT | O_WRONLY); fd_ != -1) {
      isopened_ = true;
    }
  }
  ~PosixWritableFile() {
    if (isopened_) {
      Close();
    }
  }

  void Close();
  void Append(const Slice& data);
  void Flush() { FlushBuffer(); }
  int GetFD() { return fd_; }

 private:
  void FlushBuffer() {
    WriteData(buf_, pos_);
    pos_ = 0;
  }

  void WriteData(const char* buf, std::size_t size) {
    while (size > 0) {
      int writtenBytes = ::write(fd_, buf, size);
      if (writtenBytes < 0) {
        if (errno == EINTR) {
          continue;
        }
      }
      buf += writtenBytes;
      size -= writtenBytes;
    }
  }

 private:
  int fd_;
  std::string filename_;
  bool isopened_;
  char buf_[fileBufferSize];
  std::size_t pos_;
};

class PosixSequentialFile {
 public:
  PosixSequentialFile(const Slice& filename) : filename_(filename.Data()) {
    if (fd_ = ::open(filename.Data(), O_RDONLY); fd_ != -1) {
      isopened_ = true;
    }
  }
  ~PosixSequentialFile() {
    if (isopened_) ::close(fd_);
  }

  auto Read(Slice* result, char* scratch, std::size_t nbytes) -> int;
  auto Skip(std::size_t nbytes) -> bool;
  int GetFD() { return fd_; }

 private:
  std::string filename_;
  int fd_;
  bool isopened_;
};

}  // namespace HoshinoDB
