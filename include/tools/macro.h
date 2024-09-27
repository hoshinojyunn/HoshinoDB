#pragma once

#define DISABLE_COPY(c) \
  c(const c&) = delete; \
  c& operator=(const c&) = delete;
#define DISABLE_MOVE(c) \
  c(c&&) = delete;      \
  c& operator=(c&&) = delete;

#define DISABLE_COPY_AND_MOVE(c) \
  DISABLE_COPY(c)                \
  DISABLE_MOVE(c)
