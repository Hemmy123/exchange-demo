#pragma once

#include "InternalEvents.h"
#include <cstddef>
#include <vector>

template <typename Type, std::size_t Size>
// Single threaded stand in. NOT Thread safe yet!
class SpscRing {
public:
  bool push(const Type &v) {
    if (m_buffer.size() - m_head >= Size) { // buffer full
      return false;
    }

    m_buffer.push_back(v);

    return true;
  }

  bool pop(Type &out) {
    if (m_head == m_buffer.size()) { // buffer empty
      m_buffer.clear();              // reclaim space
      m_head = 0;
      return false;
    }

    out = m_buffer[m_head++];

    return true;
  }

private:
  // TODO: using a vector as a placeholder for now. Getting the wider structure
  // in place first before I start implementing a proper SPSC Ring
  std::vector<Type> m_buffer;
  std::size_t m_head = 0;
};

using InternalEventQueue = SpscRing<InternalEvent, 4096>;
