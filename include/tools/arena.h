#pragma once
#include <gtest/gtest_prod.h>

#include <atomic>
#include <cstddef>
#include <vector>

#include "tools/macro.h"
namespace HoshinoDB {
class Arena {
 public:
  DISABLE_COPY(Arena);
  Arena();
  ~Arena();

  char* Allocate(std::size_t bytes);

  std::size_t GetMemUsage() {
    return memory_usage_.load(std::memory_order_relaxed);
  }
  FRIEND_TEST(arena_test, allocate_test);

 private:
  std::vector<char*> blocks;
  char* alloc_ptr_;
  std::size_t alloc_remains_bytes_;
  std::atomic<std::size_t> memory_usage_;
  constexpr static std::size_t blockSize = 4096;
  char* AllocateFallBack(std::size_t bytes);

  char* AllocateNewBlock(std::size_t bytes);
};

}  // namespace HoshinoDB