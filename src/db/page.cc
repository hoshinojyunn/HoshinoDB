#include "db/page.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "coding/coding.h"
#include "tools/slice.h"

namespace HoshinoDB {

bool Page::TryAdd(const Slice& key, const Slice& value) {
  assert(is_finished == false);
  auto entry_len = coding::VarintLength(key.Size()) + key.Size() +
                   coding::VarintLength(value.Size()) + value.Size();
  // 加入该kv对后，该page所占用的空间 = 当前占用空间 + entry_len +
  // 该entry的offset
  auto estimateSizeAfterAdd =
      EstimateCurSize() + entry_len + sizeof(std::uint32_t);
  // 该page剩余空间不足
  if (maxPageSize - estimateSizeAfterAdd < 0) {
    Finish();
    return false;
  }
  char buf[5];
  // append key_size and key
  auto p = coding::EncodeVarint32(buf, key.Size());
  this->buffer_.append(buf, (p - buf));
  this->buffer_.append(key.Data(), key.Size());
  // append value_size and value
  p = coding::EncodeVarint32(buf, value.Size());
  this->buffer_.append(buf, (p - buf));
  this->buffer_.append(value.Data(), value.Size());
  // 设置该entry的起始位置
  entries_offset_.push_back(buffer_.size() - entry_len);

  return true;
}

/*
    当前的entries所占空间 + entries_offset占空间 + entries_offset_num +
   有效数据长度n(前三者加起来多大)
*/
std::size_t Page::EstimateCurSize() const {
  if (!is_finished) {
    return buffer_.size() + (entries_offset_.size() * sizeof(std::uint32_t)) +
           sizeof(std::uint32_t) + sizeof(std::uint32_t);
  } else {
    return buffer_.size();
  }
}

void Page::Finish() {
  for (std::size_t i{0}; i < entries_offset_.size(); ++i) {
    coding::AppendFixed32(&buffer_, entries_offset_[i]);
  }
  coding::AppendFixed32(&buffer_, entries_offset_.size());
  is_finished = true;
}

void Page::Reset() {
  this->buffer_.clear();
  is_finished = false;
}

// 通过effective data构造一个Page
void Page::DecodeFrom(const Slice& effective_data) {
  /*
      effective_data:
      1. entries_1...entries_n
      2. entries_offset_1...entries_offset_n: uint32
      3. entries_offset_num: uint32
  */
  // 解析entries_num
  auto entries_num_offset =
      effective_data.Data() + effective_data.Size() - sizeof(std::uint32_t);
  auto entries_num = coding::DecodeFixed32(entries_num_offset);
  const char* entries_offset_ptr =
      entries_num_offset - (entries_num * sizeof(std::uint32_t));
  assert(entries_offset_.size() == 0);
  while (entries_offset_ptr < entries_num_offset) {
    auto entry_offset = coding::DecodeFixed32(entries_offset_ptr);
    entries_offset_ptr += sizeof(std::uint32_t);
    this->entries_offset_.push_back(entry_offset);
  }
  this->buffer_.assign(effective_data.Data(), effective_data.Size());
  this->entries_offset_start_ =
      this->buffer_.data() + (this->buffer_.size() - sizeof(std::uint32_t)) -
      (this->entries_offset_.size() * sizeof(std::uint32_t));
}

// 根据index解析对应的entry
Page::PageEntry Page::GetEntry(std::size_t index) const {
  assert(index < entries_offset_.size());
  std::uint32_t offset = entries_offset_[index];
  const char* entry_data = buffer_.data() + offset;
  std::uint32_t key_size;
  auto p = coding::DecodeVarint32(entry_data, entry_data + 5, &key_size);
  auto key = Slice(p, key_size);
  std::uint32_t value_size;
  p = coding::DecodeVarint32(p, p + 5, &value_size);
  auto value = Slice(p, value_size);
  return {key, value};
}

std::unique_ptr<Iterator> Page::NewIterator() const {
  return std::make_unique<PageIterator>(this->Data(), this->entries_offset_);
}

bool PageIterator::Seek(const Slice& target_key) {
  std::uint32_t left = 0;
  std::uint32_t right = entries_offset_.size();

  // 如果之前已经找过了
  if (Valid()) {
    // 先于当前的key进行对比
    auto r = cmp_.Compare(key_, target_key);
    // 当前key小于target
    if (r < 0) {
      left = cur_index_;
    } else if (r > 0) {
      right = cur_index_;
    } else {
      return true;
    }
  }
  while (left < right) {
    std::uint32_t mid = (left + right) / 2;
    std::uint32_t mid_offset = entries_offset_[mid];
    std::uint32_t key_size;
    const char* p = coding::DecodeVarint32(buffer_ + mid_offset,
                                           buffer_ + mid_offset + 5, &key_size);
    Slice mid_key = Slice(p, key_size);
    p += key_size;
    auto r = cmp_.Compare(mid_key, target_key);
    // mid_key < target
    if (r < 0) {
      left = mid + 1;
    } else if (r > 0) {
      right = mid;
    } else {
      key_ = mid_key;
      std::uint32_t value_size;
      p = coding::DecodeVarint32(p, p + 5, &value_size);
      value_ = Slice(p, value_size);
      cur_index_ = mid;
      return true;
    }
  }
  return false;
}

bool PageIterator::Next() {
  // cur_index是最后一个
  if ((std::size_t)(cur_index_ + 1) == entries_offset_.size()) {
    return false;
  }
  // assert((std::size_t)(cur_index_ + 1) < page_->GetEntriesOffset().size());
  auto [key, value] = GetEntry(cur_index_ + 1);
  key_ = key;
  value_ = value;
  cur_index_ += 1;
  return true;
}

bool PageIterator::Prev() {
  if (cur_index_ == 0) {
    return false;
  }
  auto [key, value] = GetEntry(cur_index_ - 1);
  key_ = key;
  value_ = value;
  cur_index_ -= 1;
  return true;
}

bool PageIterator::SeekFirst() {
  if (entries_offset_.size() == 0) return false;
  auto [key, value] = GetEntry(0);
  key_ = key;
  value_ = value;
  cur_index_ = 0;
  return true;
}

bool PageIterator::SeekLast() {
  auto size = entries_offset_.size();
  if (size == 0) return false;
  auto [key, value] = GetEntry(size - 1);
  key_ = key;
  value_ = value;
  cur_index_ = size - 1;
  return true;
}

}  // namespace HoshinoDB