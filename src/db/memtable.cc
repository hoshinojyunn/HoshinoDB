#include "db/memtable.h"

#include <cassert>

#include "coding/coding.h"
#include "db/key.h"
#include "tools/comparator.h"
#include "tools/slice.h"

namespace HoshinoDB {
void MemTable::Add(const Slice &key, const Slice &value, SequenceNumber seq,
                   ValueType type) {
  auto entry = Entry{key, value, seq, type};
  auto internalKey_size = entry.internalKey_size_;
  auto value_size = entry.value_size_;
  auto encoded_entry_size = coding::VarintLength(internalKey_size) +
                            internalKey_size +
                            coding::VarintLength(value_size) + value_size;
  char *buf = arena_.Allocate(encoded_entry_size);
  entry.EncodeEntry(buf);
  table_.Insert(buf);
}

bool MemTable::Get(const LookupKey &key, std::string *value) {
  auto node = table_.Find(key.MemKey().Data());
  if (node == nullptr) {
    return false;
  }
  auto entry = node->data_;
  auto encoded_entry = EncodedEntry{entry};
  auto decoded_entry = encoded_entry.DecodeEntry();
  UserKeyComparator cmp_;
  if (cmp_.Compare(key.UserKey(),
                   decoded_entry.internal_key_.ExtractUserKey()) == 0) {
    auto type = decoded_entry.internal_key_.ExtractValueType();
    if (type == typeValue) {
      value->assign(decoded_entry.value_.Data(), decoded_entry.value_size_);
      return true;
    }
    if (type == typeDeletion) {
      return true;
    }
  }
  return false;
}

}  // namespace HoshinoDB