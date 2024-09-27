#include "tools/skiplist.h"

#include <gtest/gtest.h>

#include <cstdio>

#include "tools/comparator.h"
namespace HoshinoDB {

TEST(skip_list_test, insert_test) {
  UserKeyComparator cmp;
  SkipList<const char*, decltype(cmp)> table{cmp};
  table.Insert("hello");
  table.Insert("world");
  table.Insert("hey");
  table.Insert("you");
  auto iter = table.GetIterator();
  while (auto node = iter->Next()) {
    ::printf("%s\n", node->data_);
  }
}

TEST(skip_list_test, find_and_delete_test) {
  UserKeyComparator cmp;
  SkipList<const char*, decltype(cmp)> table{cmp};
  table.Insert("hello");
  table.Insert("world");
  table.Insert("hey");
  table.Insert("you");
  // auto iter = table.GetIterator();
  auto node = table.Find("hey");
  auto node1 = table.Find("world");
  ASSERT_EQ(node->data_, "hey");
  ASSERT_EQ(node1->data_, "world");

  if (auto success = table.Delete("world")) {
    auto temp = table.Find("world");
    ASSERT_EQ(temp, nullptr);
    // ASSERT_EQ(node1, nullptr);
  }
  if (auto success = table.Delete("you")) {
    auto temp = table.Find("you");
    ASSERT_EQ(temp, nullptr);
  }
}

}  // namespace HoshinoDB