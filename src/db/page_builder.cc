#include "db/page_builder.h"

#include <cassert>
#include <memory>

#include "db/page.h"
#include "tools/slice.h"

namespace HoshinoDB {
bool PageBuilder::Add(const Slice& key, const Slice& value) {
  // 确保新加入的key比上一个key大
  assert(last_key_ == nullptr || cmp_.Compare(key, last_key_) > 0);
  auto b = page_->TryAdd(key, value);
  // 此时说明这一页空间不够存入该kv对了
  if (!b && page_->IsFinish()) {
    db_file_.WritePage(*page_);
    // page_->Reset();
    page_ = std::make_shared<Page>(++page_counter_); // 创建一个新Page
  }
  // 如果重置了page 则直接Add
  if(!b) {
    b = page_->TryAdd(key, value);
  }
  last_key_ = key;
  // 如果此时返回false，说明一整个page也存不下这个entry
  return b;
}

void PageBuilder::Reset() {
    last_key_.Clear();

}

void PageBuilder::Duration() {
  // assert(page_.IsFinish() && "PageBuilder: Page not finished when duration");
  page_->Finish();
  db_file_.WritePage(*page_);
  page_->Reset();
}

}  // namespace HoshinoDB