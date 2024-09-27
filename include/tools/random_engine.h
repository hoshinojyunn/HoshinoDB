#pragma once

#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <type_traits>

#include "tools/arena.h"
#include "tools/slice.h"

namespace HoshinoDB {
class Exception : public std::runtime_error {
 public:
  Exception(Slice s, bool print) : std::runtime_error{s.Data()} {
    if (print) {
      std::cerr << this->what();
    }
  }
};
template <class T, class v = std::void_t<>>
class RandomEngine {
 public:
  RandomEngine(T a, T b) {}
  T GetRandom() {
    throw Exception{"RandomEngine Type neither integral nor real", true};
  }
};

template <class T>
class RandomEngine<T, std::enable_if_t<std::is_integral_v<T>>> {
 public:
  RandomEngine(T a, T b) : e{std::random_device{}()}, dist{a, b} {}

  T GetRandom() { return dist(e); }

 private:
  std::default_random_engine e;
  std::uniform_int_distribution<T> dist;
};

template <class T>
class RandomEngine<T, std::enable_if_t<std::is_floating_point_v<T>>> {
 public:
  RandomEngine(T a, T b) : e{std::random_device{}()}, dist{a, b} {}

  T GetRandom() { return dist(e); }

 private:
  std::default_random_engine e;
  std::uniform_real_distribution<T> dist;
};

template <>
class RandomEngine<Slice> {
 public:
  // 生成ascii符号
  RandomEngine() : e{std::random_device{}()}, dist{0, 127} {}
  Slice GetRandom(const std::size_t len) {
    buffer_ = arena_.Allocate(len);
    for (std::size_t i{0}; i < len; ++i) {
      buffer_[i] = dist(e);
    }
    return Slice(buffer_, len);
  }

 private:
  std::default_random_engine e;
  std::uniform_int_distribution<int> dist;
  Arena arena_;
  char* buffer_;
};

}  // namespace HoshinoDB
