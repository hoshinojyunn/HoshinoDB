#include "coding/coding.h"

#include <gtest/gtest.h>

#include <cstdint>

namespace HoshinoDB {
namespace coding {
TEST(CODING_TEST, test_fixed32) {
  char dst[4];
  EncodeFixed32(dst, 114514u);
  ASSERT_EQ(DecodeFixed32(dst), 114514);
  EncodeFixed32(dst, 1919810);
  ASSERT_EQ(1919810, DecodeFixed32(dst));
  EncodeFixed32(dst, UINT32_MAX);
  ASSERT_EQ(UINT32_MAX, DecodeFixed32(dst));
}

TEST(CODING_TEST, test_fixed64) {
  char dst[8];
  EncodeFixed64(dst, UINT64_MAX);
  ASSERT_EQ(DecodeFixed64(dst), UINT64_MAX);
  EncodeFixed64(dst, 1919810);
  ASSERT_EQ(1919810, DecodeFixed64(dst));
  EncodeFixed64(dst, 0);
  ASSERT_EQ(0, DecodeFixed64(dst));
}

TEST(CODING_TEST, test_varint32) {
  char dst[5];
  std::uint32_t res;
  EncodeVarint32(dst, UINT32_MAX);
  DecodeVarint32(dst, dst + 5, &res);
  ASSERT_EQ(res, UINT32_MAX);
  EncodeVarint32(dst, 1919810);
  DecodeVarint32(dst, dst + 5, &res);
  ASSERT_EQ(1919810, res);
  EncodeVarint32(dst, 0);
  DecodeVarint32(dst, dst + 5, &res);
  ASSERT_EQ(0, res);
}
TEST(CODING_TEST, test_varin64) {
  char dst[10];
  std::uint64_t res;
  EncodeVarint64(dst, UINT64_MAX);
  DecodeVarint64(dst, dst + 10, &res);
  ASSERT_EQ(res, UINT64_MAX);
  EncodeVarint64(dst, 1919810);
  DecodeVarint64(dst, dst + 10, &res);
  ASSERT_EQ(1919810, res);
  EncodeVarint64(dst, 0);
  DecodeVarint64(dst, dst + 10, &res);
  ASSERT_EQ(0, res);
}

TEST(CODING_TEST, test_successive_encode) {
  char dst[20];
  std::uint32_t res;
  auto p = EncodeVarint64(dst, 0xab12);  // 两个字节
  p = EncodeVarint32(p, 0x2234ff);       // 三个字节
  auto decode_p = DecodeVarint32(dst, dst + 5, &res);
  ASSERT_EQ(res, 0xab12);
  decode_p = DecodeVarint32(decode_p, decode_p + 5, &res);
  ASSERT_EQ(res, 0x2234ff);
  p = EncodeVarint64(p, 0x1122334455667788);  // 八个字节
  std::uint64_t res1;
  decode_p = DecodeVarint64(decode_p, decode_p + 10, &res1);
  ASSERT_EQ(res1, 0x1122334455667788);
}

}  // namespace coding
}  // namespace HoshinoDB
