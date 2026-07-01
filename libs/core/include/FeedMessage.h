#pragma once

#include "InternalEvents.h"
#include <chrono>

struct FeedMessage {
  std::uint64_t seq; // monotonic, for gap detection
  std::chrono::system_clock::time_point timeStamp; // shared a batch of events
  InternalEvent payload; // the variant: Trade / Added / Removed
};
