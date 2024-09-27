#pragma once

#include <memory>

#include "db/files.h"
#include "db/memtable.h"
#include "db/page.h"
namespace HoshinoDB {
class PageReader {
 public:
  PageReader(const Slice& filename) : file_(filename) {
    buf_ = arena_.Allocate(Page::maxPageSize);
  }
  std::shared_ptr<Page> ReadPage(pageid_t pageid);

 private:
  PosixReadableFile file_;
  Arena arena_;
  char* buf_;
};
}  // namespace HoshinoDB