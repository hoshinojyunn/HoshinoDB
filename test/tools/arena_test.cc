#include "tools/arena.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdio>
#include <cstring>

#include "tools/random_engine.h"

namespace HoshinoDB {
TEST(arena_test, allocate_test) {
  Arena arena{};
  // 1.first test: 2048 > 1024 会独立成块
  char *buf = arena.Allocate(2048);
  char garbage[2048];
  RandomEngine<unsigned char> rnd{0, 255};
  for (int i{0}; i < 2048; ++i) {
    garbage[i] = rnd.GetRandom();
  }
  ::memcpy(buf, garbage, 2048);

  // 2.second test: 新块
  char *buf1 = arena.Allocate(1024);
  ::memcpy(buf1, "hello world", ::strlen("hello world"));
  // 此时buf2应该在buf1的块中，且该块还剩1字节的内存
  char *buf2 = arena.Allocate(3071);
  ::memcpy(buf2, "this is arena", ::strlen("this is arena"));

  // 3.third test: buf1所在块装不下，会另外开新块，
  char *buf3 = arena.Allocate(2);
  buf3[0] = 'a';
  buf3[1] = 'b';
  ASSERT_EQ(arena.blocks.size(), 3);
  for (std::size_t i{0}; i < arena.blocks.size(); ++i) {
    if (i == 0) {
      ::printf("test first block\n");
      auto r = ::memcmp(arena.blocks[i], garbage, 2048);
      ASSERT_EQ(r, 0);
    } else if (i == 1) {
      ::printf("test second block\n");
      auto r =
          ::memcmp(arena.blocks[i], "hello world", ::strlen("hello world"));
      ASSERT_EQ(r, 0);
      r = ::memcmp(buf1 + 1024, buf2, ::strlen("this is arena"));
      ASSERT_EQ(r, 0);
    } else if (i == 2) {
      ::printf("test third block\n");
      auto r = ::memcmp(arena.blocks[i], "ab", ::strlen("ab"));
      ASSERT_EQ(r, 0);
    }
  }
}

}  // namespace HoshinoDB