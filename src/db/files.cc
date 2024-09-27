#include "db/files.h"

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstring>

#include "tools/slice.h"

namespace HoshinoDB {
auto PosixReadableFile::Valid() -> bool { return isopened_; }

auto PosixReadableFile::Close() -> bool {
  ::fsync(fd_);
  if (int res = ::close(fd_); res != -1) {
    isopened_ = false;
    return true;
  }
  return false;
}

/*
    return: 成功读取的字节数/出错返回-1/读到EOF返回0
*/
auto PosixReadableFile::Read(Slice* result, char* const scratch, int offset,
                             std::size_t nbytes) -> int {
  int n = ::pread(fd_, scratch, nbytes, offset);

  *result = Slice(scratch, (n < 0) ? 0 : n);
  return n;
}

auto PosixWritableFile::Append(const Slice& data) -> void {
  auto write_data = data.Data();
  auto write_size = data.Size();
  auto copy_size = std::min(write_size, fileBufferSize - pos_);
  ::memcpy(buf_ + pos_, write_data, copy_size);
  write_data += copy_size;
  write_size -= copy_size;
  pos_ += copy_size;
  if (write_size == 0) {
    // FlushBuffer();
    return;
  }
  // buf空间不够，先Flush到磁盘
  FlushBuffer();
  // 剩下的数据递归使用Append写到磁盘
  auto lastData = Slice(write_data, write_size);
  Append(lastData);
}

auto PosixWritableFile::Close() -> void {
  if (fd_ >= 0) FlushBuffer();
  ::close(fd_);
}

auto PosixSequentialFile::Read(Slice* result, char* scratch,
                               std::size_t nbytes) -> int {
  int res;
  while (true) {
    int read_bytes = ::read(fd_, scratch, nbytes);
    if (read_bytes < 0) {
      if (errno == EINTR) {
        continue;
      }
      res = -1;
      break;
    }
    *result = Slice(scratch, read_bytes);
    res = read_bytes;
    break;
  }
  return res;
}

auto PosixSequentialFile::Skip(std::size_t nbytes) -> bool {
  // lseek返回当前文件的操作position，返回-1表示失败
  if (::lseek(fd_, nbytes, SEEK_CUR) == static_cast<off_t>(-1)) {
    return false;
  }
  return true;
}

}  // namespace HoshinoDB
