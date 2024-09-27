#include "tools/random_engine.h"

#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>

#include <cstdint>
#include <cstdio>

namespace HoshinoDB {
TEST(random_engine_test, int_engine_test) {
  RandomEngine<std::uint8_t> engine{0, 100};
  for (auto i{0}; i < 100; ++i) {
    auto n = engine.GetRandom();
    ::printf("%d->%d\n", i, n);
  }
  auto func = []() {
    RandomEngine<const char*> r{"hello", "world"};
    r.GetRandom();  // 如果仅仅创建对象r但不使用的话，编译器会优化它不去实例化
  };

  ASSERT_THROW(func(), Exception);
}

TEST(random_engine_test, real_engine_test) {
  RandomEngine<float> engine{0.0, 1.0};
  for (auto i{0}; i < 100; ++i) {
    auto n = engine.GetRandom();
    ::printf("%f->%f\n", (float)i, n);
  }
}
}  // namespace HoshinoDB
