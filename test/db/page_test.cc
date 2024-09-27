#include <gtest/gtest.h>

#include <cstdint>
#include <fstream>

#include "coding/coding.h"
#include "db/page_reader.h"
#include "db/page_writer.h"
#include "tools/slice.h"

namespace HoshinoDB {

TEST(page_test, page_write_test) {
  // 清空文件
  std::ofstream ofs{"test.db", std::ios_base::out};
  ofs.close();

  PageWriter writer{"test.db"};
  Page page1{0};
  Page page2{1};
  Page page3{3};
  page1.TryAdd("abcd", "efg");
  page1.TryAdd("hey", "you");

  page2.TryAdd("hello", "world");
  page3.TryAdd("hello", "hoshino");
  page1.Finish();
  page2.Finish();
  page3.Finish();
  writer.WritePage(page1);
  writer.WritePage(page2);
  writer.WritePage(page3);

  PageReader reader{"test.db"};
  // page0
  auto page0 = reader.ReadPage(0);
  auto iter = page0->NewIterator();
  auto b = iter->Seek("hey");
  ASSERT_TRUE(b && iter->Valid());
  ASSERT_EQ(iter->key(), "hey");
  ASSERT_EQ(iter->value(), "you");
  // page1
  auto read_page1 = reader.ReadPage(1);
  auto iter1 = read_page1->NewIterator();
  b = iter1->Seek("hello");
  ASSERT_TRUE(b && iter1->Valid());
  ASSERT_EQ(iter1->key(), "hello");
  ASSERT_EQ(iter1->value(), "world");
  // page2
  auto read_page2 = reader.ReadPage(1);
  auto iter2 = read_page2->NewIterator();
  b = iter2->Seek("hello");
  ASSERT_TRUE(b && iter2->Valid());
  ASSERT_EQ(iter2->key(), "hello");
  ASSERT_EQ(iter2->value(), "world");
}


TEST(page_test, page_builder_test) {
  // 清空文件
  std::ofstream ofs{"test.db", std::ios_base::out};
  ofs.close();

  PageWriter writer{"test.db"};
}
}  // namespace HoshinoDB
