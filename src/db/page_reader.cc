

#include "db/page_reader.h"

#include <cstdint>
#include <memory>

#include "coding/coding.h"
#include "db/page.h"
#include "tools/slice.h"
namespace HoshinoDB {

std::shared_ptr<Page> PageReader::ReadPage(pageid_t pageid) {
  Slice res{};
  if (auto i = file_.Read(&res, buf_, (pageid * Page::maxPageSize),
                          Page::maxPageSize);
      i > 0) {
    auto effective_data_size = coding::DecodeFixed32(res.Data());
    Slice effective_data =
        Slice(res.Data() + sizeof(std::uint32_t), effective_data_size);
    auto page = std::make_shared<Page>(pageid);
    // TODO: 构造Page
    page->DecodeFrom(effective_data);
    return page;
  }
  return nullptr;
}

}  // namespace HoshinoDB