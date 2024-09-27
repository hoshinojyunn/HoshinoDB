#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
namespace HoshinoDB {
class Slice {
 public:
  Slice() : data_(""), size_(0) {}
  Slice(const char* s) : data_(s), size_(strlen(s)) {}
  Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}
  Slice(const char* s, std::size_t size) : data_(s), size_(size) {}
  Slice(const Slice& s) = default;
  Slice& operator=(const Slice& s) = default;

  const char* Data() const { return data_; }
  std::size_t Size() const { return size_; }

  char operator[](std::size_t n) const {
    assert(n < size_ && "In Slice: index should smaller than size");
    return data_[n];
  }

  bool Empty() const { return size_ == 0; }

  void Clear() {
    data_ = "";
    size_ = 0;
  }

  void DropPrefix(std::size_t n) {
    assert(n <= size_ && "In Slice: drop byte num should smaller than size");
    data_ += n;
    size_ -= n;
  }

  std::string ToString() const { return std::string{data_, size_}; }

  bool StartsWith(const Slice& s) const {
    if (s.Size() > this->Size()) return false;
    return memcmp(this->Data(), s.Data(), s.Size()) == 0;
  }

  int Compare(const Slice& s) const {
    const std::size_t min_len =
        this->Size() > s.Size() ? s.Size() : this->Size();
    int res = memcmp(this->Data(), s.Data(), min_len);
    // 如果前缀相同，返回长的那个大过短的
    if (res == 0) {
      if (this->Size() > s.Size()) {
        return +1;
      } else if (this->Size() < s.Size()) {
        return -1;
      }
    }
    return res;
  }

 private:
  const char* data_;
  std::size_t size_;
};

inline bool operator==(const Slice& lhs, const Slice& rhs) {
  return (lhs.Size() == rhs.Size() &&
          memcmp(lhs.Data(), rhs.Data(), lhs.Size()) == 0);
}

inline bool operator!=(const Slice& lhs, const Slice& rhs) {
  return !(lhs == rhs);
}
}  // namespace HoshinoDB
