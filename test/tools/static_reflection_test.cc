#include "tools/static_reflection.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <string>

template <class T>
struct Base {
  virtual void hello(T t) = 0;
};

template <class T>
struct Derived : public Base<T> {
  void hello(T t) override { ::printf("hello\n"); }
};

struct X {};

namespace HoshinoDB {
TEST(static_reflection_test, is_base_of_template_test) {
  auto b = is_base_of_template<Base, Derived<int>>::value;
  auto b1 = is_base_of_template<Base, Derived<double>>::value;
  auto b2 = is_base_of_template<Base, Derived<std::string>>::value;
  ASSERT_EQ(b, true);
  ASSERT_EQ(b1, true);
  ASSERT_EQ(b2, true);
  auto b3 = is_base_of_template<Base, X>::value;
  ASSERT_EQ(b3, false);
}
}  // namespace HoshinoDB