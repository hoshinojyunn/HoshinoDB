#include "db/key.h"

#include <cstdint>
#include <cstring>

#include "coding/coding.h"
#include "tools/slice.h"

namespace HoshinoDB {

/*
    entry:
    1. internalKey_size: varint
    2. internal_key
    3. value_size: varint
    4. value
*/

void EncodedInternalKey::EncodeInternalKey(std::string *dest,
                                           const InternalKey &internalKey) {
  dest->append(internalKey.user_key_.Data(), internalKey.user_key_.Size());
  coding::AppendFixed64(dest, internalKey.GetTag());
}

Slice EncodedInternalKey::ExtractUserKey() const {
  return Slice(content_.data(), content_.size() - 8);
}

std::uint64_t EncodedInternalKey::ExtractTag() const {
  return coding::DecodeFixed64(content_.data() + (content_.size() - 8));
}

SequenceNumber EncodedInternalKey::ExtractSequenceNum() const {
  auto tag = ExtractTag();
  return tag >> 8;
}

ValueType EncodedInternalKey::ExtractValueType() const {
  auto tag = ExtractTag();
  ValueType type = static_cast<ValueType>(tag & 0xff);
  if (type == typeValue) {
    return typeValue;
  } else if (type == typeDeletion) {
    return typeDeletion;
  }
  return typeInvalid;
}

LookupKey::LookupKey(Slice user_key, SequenceNumber seq) {
  auto internalKey = EncodedInternalKey{user_key, seq, typeValue};
  char buf[5];
  auto p = coding::EncodeVarint32(buf, internalKey.Size());
  char *scratch = new char[internalKey.Size() + (p - buf)];
  // encode key_size
  ::memcpy(scratch, buf, (p - buf));
  // encode internal_key
  ::memcpy(scratch + (p - buf), internalKey.Data(), internalKey.Size());
  start_ = scratch;
  key_start_ = scratch + (p - buf);
  end_ = scratch + (p - buf) + internalKey.Size();
}

LookupKey::~LookupKey() { delete[] start_; }

void Entry::EncodeEntry(char *res) {
  // append internalKey_size: varint32
  auto p = coding::EncodeVarint32(res, this->internalKey_size_);
  // append internalKey
  std::memcpy(p, this->internal_key_.Data(), this->internalKey_size_);
  p += this->internalKey_size_;
  // append value_size
  p = coding::EncodeVarint32(p, this->value_size_);
  // append value
  std::memcpy(p, this->value_.Data(), this->value_size_);
}

Entry EncodedEntry::DecodeEntry() {
  std::uint32_t key_size;
  std::uint32_t value_size;
  auto p =
      coding::DecodeVarint32(this->content_, this->content_ + 5, &key_size);
  auto internal_key_data = Slice(p, key_size);
  auto internal_key = EncodedInternalKey{internal_key_data};
  p += key_size;
  p = coding::DecodeVarint32(p, p + 5, &value_size);
  auto value = Slice(p, value_size);
  return Entry{internal_key.ExtractUserKey(), value,
               internal_key.ExtractSequenceNum(),
               internal_key.ExtractValueType()};
}

}  // namespace HoshinoDB