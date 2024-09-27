#pragma once
#include <cstddef>
#include <memory>

#include "db/page.h"
#include "db/page_writer.h"
#include "tools/comparator.h"
#include "tools/slice.h"

namespace HoshinoDB {

/*
    MemTable应使用PageBuilder进行持久化
*/
class PageBuilder {
 public:
  PageBuilder(pageid_t pageid, const Slice& db_file_path)
      : db_file_(db_file_path), page_(std::make_shared<Page>(pageid)), page_counter_(0) {}
  PageBuilder(std::shared_ptr<Page> page, const Slice& db_file_path)
      : db_file_(db_file_path), page_(page), page_counter_(0) {}

  bool Add(const Slice& key, const Slice& value);
  void Duration();
  void Reset();

 private:
  PageWriter db_file_;
  Slice last_key_;
  std::shared_ptr<Page> page_;
  UserKeyComparator cmp_;
  std::size_t page_counter_;
};

}  // namespace HoshinoDB