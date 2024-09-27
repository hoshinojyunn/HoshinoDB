
#include "db/page_writer.h"

#include <unistd.h>

#include <cassert>

#include "coding/coding.h"
#include "db/page.h"
namespace HoshinoDB {

bool PageWriter::WritePage(const Page &page) {
  assert(page.IsFinish() && "PageWriter: write unfinished page!");
  if (::lseek(file_.GetFD(), page.GetPageID() * Page::maxPageSize, SEEK_SET) ==
      static_cast<off_t>(-1)) {
    return false;
  }
  char buf[4];
  // append
  // page finished后，EstimateCurSize返回构筑好的buffer_的长度
  coding::EncodeFixed32(buf, page.EstimateCurSize());
  file_.Append(Slice(buf, 4));
  file_.Append(Slice(page.Data(), page.EstimateCurSize()));
  file_.Flush();  // 将这页输出到磁盘
  return true;
}
}  // namespace HoshinoDB