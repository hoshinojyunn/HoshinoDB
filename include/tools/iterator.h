#pragma once

#include "tools/macro.h"
#include "tools/slice.h"
namespace HoshinoDB {

class Iterator {
 public:
  DISABLE_COPY(Iterator);
  Iterator() = default;
  virtual ~Iterator(){};
  virtual bool Valid() const = 0;
  virtual bool Seek(const Slice& target) = 0;
  virtual bool SeekFirst() = 0;
  virtual bool SeekLast() = 0;
  virtual bool Next() = 0;
  virtual bool Prev() = 0;
  virtual Slice key() const = 0;
  virtual Slice value() const = 0;
};

}  // namespace HoshinoDB
