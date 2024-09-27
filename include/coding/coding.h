#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
namespace HoshinoDB {
namespace coding {
inline void EncodeFixed32(char* dst, std::uint32_t num) {
  dst[0] = static_cast<std::uint8_t>(num);
  dst[1] = static_cast<std::uint8_t>(num >> 8);
  dst[2] = static_cast<std::uint8_t>(num >> 16);
  dst[3] = static_cast<std::uint8_t>(num >> 24);
}

inline std::uint32_t DecodeFixed32(const char* src) {
  auto ptr = reinterpret_cast<const std::uint8_t*>(src);
  std::uint32_t res = 0;
  res |= static_cast<std::uint32_t>(ptr[0]);
  res |= static_cast<std::uint32_t>(ptr[1]) << 8;
  res |= static_cast<std::uint32_t>(ptr[2]) << 16;
  res |= static_cast<std::uint32_t>(ptr[3]) << 24;
  return res;
}
inline void EncodeFixed64(char* dst, std::uint64_t num) {
  int index = 0;
  for (std::size_t shift = 0; shift <= 56; shift += 8) {
    dst[index++] = static_cast<std::uint8_t>(num >> shift);
  }
}
inline std::uint64_t DecodeFixed64(const char* src) {
  auto ptr = reinterpret_cast<const std::uint8_t*>(src);
  std::uint64_t res = 0;
  int index = 0;
  for (std::size_t shift = 0; shift <= 56; shift += 8) {
    std::uint64_t byte = ptr[index++];
    res |= byte << shift;
  }
  return res;
}
inline char* EncodeVarint64(char* dst, std::uint64_t num) {
  constexpr int CBit = 128;
  auto ptr = reinterpret_cast<std::uint8_t*>(dst);
  while (num >= CBit) {
    *(ptr++) = (num & 0x7f) | CBit;
    num >>= 7;
  }
  *(ptr++) = num & 0x7f;
  return reinterpret_cast<char*>(ptr);
}
inline char* EncodeVarint32(char* dst, std::uint32_t num) {
  return EncodeVarint64(dst, num);
}

inline const char* DecodeVarint64(const char* src, const char* limit,
                                  std::uint64_t* res) {
  constexpr int CBit = 128;
  *res = 0;
  auto ptr = reinterpret_cast<const std::uint8_t*>(src);
  auto ptr_limit = reinterpret_cast<const std::uint8_t*>(limit);
  int shift = 0;
  while (*ptr >= CBit && ptr < ptr_limit) {
    std::uint64_t byte = *(ptr++);  // 要先把*ptr转为uint64，否则后面左移会溢出
    *res |= (byte & 0x7f) << shift;
    shift += 7;
  }
  if (ptr < ptr_limit) {
    std::uint64_t byte = *(ptr++);
    *res |= (byte & 0x7f) << shift;
  }
  return reinterpret_cast<const char*>(ptr);
}

inline const char* DecodeVarint32(const char* src, const char* limit,
                                  std::uint32_t* res) {
  std::uint64_t temp_res;
  auto p = DecodeVarint64(src, limit, &temp_res);
  *res = static_cast<std::uint32_t>(temp_res);
  return p;
}

inline void AppendFixed32(std::string* s, std::uint32_t num) {
  char buf[4];
  EncodeFixed32(buf, num);
  s->append(buf, sizeof(buf));
}

inline void AppendFixed64(std::string* s, std::uint64_t num) {
  char buf[8];
  EncodeFixed64(buf, num);
  s->append(buf, sizeof(buf));
}

inline std::size_t VarintLength(std::uint64_t num) {
  char buf[10];
  auto p = EncodeVarint64(buf, num);
  return (std::size_t)(p - buf);
}

}  // namespace coding
}  // namespace HoshinoDB