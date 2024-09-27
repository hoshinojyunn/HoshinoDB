#include "db/memtable.h"

#include <gtest/gtest.h>

#include "db/key.h"
#include "tools/comparator.h"

namespace HoshinoDB {
TEST(memtable_test, add_test) {
  MemTable memTable{EntryComparator{}};
  memTable.Add("hello", "world", 0, typeValue);
  memTable.Add("hey", "you", 1, typeValue);
  memTable.Add("add", "fruit", 2, typeValue);
  // TODO: 
  std::string res;
  bool b = memTable.Get(LookupKey{"hello", 0}, &res);
  ASSERT_TRUE(b);
  ASSERT_EQ(res, "world");
  b = memTable.Get(LookupKey{"add", 2}, &res);
  ASSERT_EQ(res, "fruit");
  b = memTable.Get(LookupKey{"hey", 1}, &res);
  ASSERT_EQ(res, "you");
  res.clear();
  b = memTable.Get(LookupKey{"hello", 1}, &res);
  ASSERT_FALSE(b);
  ASSERT_NE(res, "world");
  memTable.Add("hello", "world123", 3, typeValue);
  memTable.Get(LookupKey{"hello", 0}, &res);
  ASSERT_EQ(res, "world");
  memTable.Get(LookupKey{"hello", 3}, &res);
  ASSERT_EQ(res, "world123");
}
}  // namespace HoshinoDB