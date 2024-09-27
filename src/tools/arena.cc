#include "tools/arena.h"

#include <atomic>
#include <cassert>
#include <cstddef>

namespace HoshinoDB {
Arena::Arena()
    : alloc_ptr_(nullptr), alloc_remains_bytes_(0), memory_usage_(0) {}
Arena::~Arena() {
  for (auto block : blocks) {
    delete[] block;
  }
}

char* Arena::Allocate(std::size_t bytes) {
  assert(bytes > 0 && "Arena: allocate bytes should greater than 0");
  if (bytes <= alloc_remains_bytes_) {
    auto res = alloc_ptr_;
    alloc_ptr_ += bytes;
    alloc_remains_bytes_ -= bytes;
    return res;
  }
  return AllocateFallBack(bytes);
}

char* Arena::AllocateFallBack(std::size_t bytes) {
  /*
      如果要申请的空间大于1024个字节，那么就直接给他申请bytes大小的空间让它单独成一个block
      原先的block继续用
  */
  if (bytes > Arena::blockSize / 4) {
    return AllocateNewBlock(bytes);
  }
  /*
      如果要申请的空间又比1024个字节小，而当前的block又不够空间分给它，此时即使抛弃当前block没用完的部分
      也不会造成过多浪费，直接丢弃。开一个新block存
  */
  char* res = AllocateNewBlock(Arena::blockSize);
  alloc_ptr_ = res + bytes;
  alloc_remains_bytes_ = Arena::blockSize - bytes;
  return res;
}

char* Arena::AllocateNewBlock(std::size_t bytes) {
  char* res = new char[bytes];
  blocks.push_back(res);
  // 内存的使用 = 申请的bytes + 保存在blocks中的指针
  memory_usage_.fetch_add(bytes + sizeof(char*), std::memory_order_relaxed);
  return res;
}
}  // namespace HoshinoDB