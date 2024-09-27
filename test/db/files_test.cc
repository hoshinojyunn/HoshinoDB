#include "db/files.h"

#include <gtest/gtest.h>

#include <fstream>
#include <ios>
#include <memory>
#include <string>

#include "tools/random_engine.h"
#include "tools/slice.h"

namespace HoshinoDB {
TEST(files_test, posix_writableFile_test) {
  // 清空文件
  std::ofstream ofs{"test.db", std::ios_base::out};
  ofs.close();

  RandomEngine<std::uint8_t> rnd{0, 255};
  PosixWritableFile file("test.db");
  std::string t1 = "abcdefg";
  file.Append(t1);
  auto t2 = std::make_shared<std::string>();
  for (int i = 0; i < 65536; ++i) {
    t2->push_back(static_cast<char>(rnd.GetRandom()));
  }
  file.Append(*t2);
  file.Close();

  PosixReadableFile file1("test.db");
  Slice s;
  constexpr int bufferSize = 1024;
  char buf[bufferSize];
  file1.Read(&s, buf, 0, t1.size());
  ASSERT_EQ(s, "abcdefg");
  file1.Read(&s, buf, 2, 5);
  ASSERT_EQ(s, "cdefg");

  int offset = t1.size();
  auto r2 = std::make_shared<std::string>();
  int n;
  while ((n = file1.Read(&s, buf, offset, bufferSize)) != 0) {
    r2->append(s.Data(), s.Size());
    offset += n;
  }
  ASSERT_EQ(*r2, *t2);
}

TEST(files_test, posix_sequentialFile_test) {
  // 清空文件
  std::ofstream ofs{"test.db", std::ios_base::out};
  ofs.close();

  RandomEngine<std::uint8_t> rnd{0, 255};
  PosixWritableFile file("test.db");
  std::string t1 = "abcdefg";
  file.Append(t1);
  auto t2 = std::make_shared<std::string>();
  for (int i = 0; i < 65536; ++i) {
    t2->push_back(static_cast<char>(rnd.GetRandom()));
  }
  file.Append(*t2);
  file.Close();

  PosixSequentialFile file1{"test.db"};
  Slice s;
  constexpr int bufferSize = 1024;
  char buf[bufferSize];
  file1.Read(&s, buf, 5);
  ASSERT_EQ(s, "abcde");
  file1.Skip(2);
  int n;
  auto r2 = std::make_shared<std::string>();
  while ((n = file1.Read(&s, buf, bufferSize)) != 0) {
    r2->append(s.Data(), s.Size());
  }
  ASSERT_EQ(*t2, *r2);
}

}  // namespace HoshinoDB