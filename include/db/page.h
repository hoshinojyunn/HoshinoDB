#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "tools/comparator.h"
#include "tools/iterator.h"
#include "tools/slice.h"
namespace HoshinoDB {
using pageid_t = std::uint64_t;

/*
    _____________________________
   |        page header          |
    ￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣
    _____________________________
   |key_size|key|value_size|value|
    ￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣
                 ·
                 ·
                 ·
    _____________________________ <----metadata
   |        entry1 offset        |
    ￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣
    _____________________________
   |        entry2 offset        |
    ￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣
                 ·
                 ·
    _____________________________
   |        offset number        |
    ￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣
    Page中Add进来的entry的key应该无重复，SequenceNumer版本号问题应当在调用page_builder前解决，
    Page加进来的数据应在page_builder中保证有序性
*/

class Page;

class PageIterator : public Iterator {
 public:
  PageIterator(const char* data, const std::vector<std::uint32_t>& offset)
      : buffer_(data), entries_offset_(offset), cur_index_(-1) {}
  ~PageIterator() = default;
  bool Seek(const Slice& target) override;
  bool Next() override;
  bool Prev() override;
  Slice key() const override { return key_; }
  Slice value() const override { return value_; }
  bool Valid() const override { return cur_index_ != -1; }
  bool SeekFirst() override;
  bool SeekLast() override;

 private:
  Slice key_;
  Slice value_;
  const char* buffer_;
  const std::vector<std::uint32_t>& entries_offset_;

  int cur_index_;
  const UserKeyComparator cmp_;
  std::pair<Slice, Slice> GetEntry(std::size_t index) {
    assert(index < entries_offset_.size());
    std::uint32_t offset = entries_offset_[index];
    const char* entry_data = buffer_ + offset;
    std::uint32_t key_size;
    auto p = coding::DecodeVarint32(entry_data, entry_data + 5, &key_size);
    auto key = Slice(p, key_size);
    std::uint32_t value_size;
    p = coding::DecodeVarint32(p, p + 5, &value_size);
    auto value = Slice(p, value_size);
    return {key, value};
  }
};

class Page {
 public:
  constexpr static std::size_t maxPageSize = 1024 * 32;  // 32kB
  struct PageEntry {
    Slice key;
    Slice value;
  };
  Page(pageid_t pageid) : page_id_(pageid), is_finished(false) {}
  /*
    TryAdd尝试加入键值对 key:value
    若加入该键值对后使得当前Page的有效数据容量大于maxPageSize，
    则调用Finish并返回false，否则应当加入该键值对到当前page中
  */
  bool TryAdd(const Slice& key, const Slice& value);
  void Finish();
  void Reset();
  std::size_t EstimateCurSize() const;
  void DecodeFrom(const Slice& effective_data);
  bool IsFinish() const { return is_finished; }
  const char* Data() const { return buffer_.data(); }
  pageid_t GetPageID() const { return page_id_; }
  std::unique_ptr<Iterator> NewIterator() const;
  const std::vector<std::uint32_t>& GetEntriesOffset() const {
    return this->entries_offset_;
  }
  std::uint32_t GetOffset(std::size_t index) const {
    return entries_offset_[index];
  }
  PageEntry GetEntry(std::size_t index) const;

 private:
  pageid_t page_id_;
  std::string
      buffer_;  // Finishi后，存储entries + entries_offset + entries_offset_num
  const char* entries_offset_start_;
  std::vector<std::uint32_t> entries_offset_;
  bool is_finished;
};

}  // namespace HoshinoDB