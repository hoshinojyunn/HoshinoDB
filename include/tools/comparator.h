#pragma once
#include <cstdint>

#include "coding/coding.h"
#include "db/key.h"
#include "tools/slice.h"

namespace HoshinoDB {

template <class T>
class Comparator {
 public:
  Comparator() = default;
  virtual auto Compare(const T& lhs, const T& rhs) const -> int = 0;
};

class UserKeyComparator : public Comparator<Slice> {
 public:
  UserKeyComparator() = default;
  auto Compare(const Slice& lhs, const Slice& rhs) const -> int override {
    return lhs.Compare(rhs);
  }
};

class InternalKeyComparator {
 public:
  InternalKeyComparator() = default;

  auto Compare(const InternalKey& key1, const InternalKey& key2) const -> int {
    int r = cmp_.Compare(key1.user_key_, key2.user_key_);
    if (r == 0) {
      std::uint64_t tag1 = key1.GetTag();
      std::uint64_t tag2 = key2.GetTag();
      if (tag1 > tag2) {
        return +1;
      } else if (tag1 < tag2) {
        return -1;
      }
    }
    return r;
  }

 private:
  UserKeyComparator cmp_;
};

class EncodedInternalKeyComparator {
 public:
  EncodedInternalKeyComparator() = default;
  auto Compare(const EncodedInternalKey& key1,
               const EncodedInternalKey& key2) const -> int {
    auto internalKey1 =
        InternalKey(key1.ExtractUserKey(), key1.ExtractSequenceNum(),
                    key1.ExtractValueType());
    auto internalKey2 =
        InternalKey(key2.ExtractUserKey(), key2.ExtractSequenceNum(),
                    key2.ExtractValueType());
    return cmp_.Compare(internalKey1, internalKey2);
  }

 private:
  InternalKeyComparator cmp_;
};

class EntryComparator {
 public:
  // 可以传入entry或lookupKey
  auto Compare(const char* entry1, const char* entry2) const -> int {
    return cmp_.Compare(ExtractInternalKey(entry1), ExtractInternalKey(entry2));
  }

 private:
  InternalKeyComparator cmp_;
  InternalKey ExtractInternalKey(const char* entry) const {
    std::uint32_t key_size;
    auto p1 = coding::DecodeVarint32(entry, entry + 5, &key_size);
    auto internalKey = Slice(p1, key_size);
    auto user_key = Slice(internalKey.Data(), internalKey.Size() - 8);
    auto tag_src = Slice(internalKey.Data() + (internalKey.Size() - 8), 8);
    auto tag = coding::DecodeFixed64(tag_src.Data());
    ValueType type;
    ValueType temp_type = static_cast<ValueType>(tag & 0xff);
    if (temp_type == typeValue) {
      type = typeValue;
    } else if (temp_type == typeDeletion) {
      type = typeDeletion;
    } else {
      type = typeInvalid;
    }
    SequenceNumber seq = (tag >> 8);
    return InternalKey{user_key, seq, type};
  }
};

}  // namespace HoshinoDB