#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

#include "tools/macro.h"
#include "tools/slice.h"
namespace HoshinoDB {
using SequenceNumber = std::uint64_t;
// 7byte能存的最大值
constexpr static SequenceNumber maxSequenceNumber = ((1ull << 56) - 1);

enum ValueType { typeDeletion = 0x0, typeValue = 0x1, typeInvalid = 0x2 };

struct InternalKey {
  Slice user_key_;
  SequenceNumber seq_;
  ValueType type_;
  InternalKey(const Slice &user_key, const SequenceNumber &seq,
              const ValueType &type)
      : user_key_(user_key), seq_(seq), type_(type) {}
  std::uint64_t GetTag() const {
    assert(seq_ <= maxSequenceNumber &&
           "InternalKey: sequence number exceed maxSequenceNumber");
    return (seq_ << 8) | type_;
  }
};

class EncodedInternalKey {
 public:
  EncodedInternalKey() = default;
  EncodedInternalKey(const Slice &user_key, const SequenceNumber &seq,
                     const ValueType &type) {
    EncodeInternalKey(&this->content_, InternalKey(user_key, seq, type));
  }
  EncodedInternalKey(Slice data) {
    content_.clear();
    content_.append(data.Data(), data.Size());
  }
  std::size_t Size() const { return content_.size(); }
  const char *Data() const { return content_.data(); }
  Slice ExtractUserKey() const;
  SequenceNumber ExtractSequenceNum() const;
  ValueType ExtractValueType() const;
  std::uint64_t ExtractTag() const;
  Slice GetContent() const { return content_; }

 private:
  std::string content_;
  void EncodeInternalKey(std::string *dest, const InternalKey &internalKey);
};

// 传入memtable中进行查找使用的key，由key_size、user_key、tag组成
class LookupKey {
 public:
  DISABLE_COPY(LookupKey);
  LookupKey(Slice user_key, SequenceNumber seq);
  ~LookupKey();
  Slice MemKey() const { return Slice(start_, (end_ - start_)); }
  Slice InternalKey() const { return Slice(key_start_, (end_ - key_start_)); }
  Slice UserKey() const { return Slice(key_start_, (end_ - key_start_ - 8)); }

 private:
  const char *start_;
  const char *key_start_;
  const char *end_;
};

struct Entry {
  std::size_t internalKey_size_;
  EncodedInternalKey internal_key_;
  std::size_t value_size_;
  Slice value_;
  Entry(const Slice &key, const Slice &value, const SequenceNumber &seq,
        const ValueType &type) {
    internal_key_ = EncodedInternalKey{key, seq, type};
    internalKey_size_ = internal_key_.Size();
    value_size_ = value.Size();
    value_ = value;
  }
  void EncodeEntry(char *res);
};

class EncodedEntry {
 public:
  EncodedEntry(const char *entry) : content_(entry) {}
  Entry DecodeEntry();

 private:
  const char *content_;
};

}  // namespace HoshinoDB