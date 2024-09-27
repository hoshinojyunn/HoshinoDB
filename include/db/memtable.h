#pragma once
#include "db/key.h"
#include "tools/arena.h"
#include "tools/comparator.h"
#include "tools/skiplist.h"

namespace HoshinoDB {
class MemTable {
 public:
  MemTable(EntryComparator cmp) : table_(cmp) {}

  void Add(const Slice& key, const Slice& value, SequenceNumber seq,
           ValueType type);
  bool Get(const LookupKey& key, std::string* value);

 private:
  EntryComparator cmp_;
  SkipList<const char*, EntryComparator> table_;
  Arena arena_;
};

}  // namespace HoshinoDB
