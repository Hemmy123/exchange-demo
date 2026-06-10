#pragma once

#define NON_COPYABLE_NON_MOVEABLE(Type)                                        \
  Type(const Type &) = delete;                                                 \
  Type &operator=(const Type &) = delete;                                      \
  Type(Type &&) = delete;                                                      \
  Type &operator=(Type &&) = delete;

#define MOVE_ONLY(Type)                                                        \
  Type(const Type &) = delete;                                                 \
  Type &operator=(const Type &) = delete;                                      \
  Type(Type &&) = default;                                                     \
  Type &operator=(Type &&) = default;
