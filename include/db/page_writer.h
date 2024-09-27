#pragma once
#include "db/files.h"
#include "db/page.h"
namespace HoshinoDB {
class PageWriter {
 public:
  PageWriter(const Slice& filename) : file_(filename) {}
  bool WritePage(const Page& page);

 private:
  PosixWritableFile file_;
};

}  // namespace HoshinoDB