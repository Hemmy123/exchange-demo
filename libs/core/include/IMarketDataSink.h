#pragma once

#include "FeedMessage.h"

struct IMarketDataSink {
  virtual void Publish(const FeedMessage &msg) = 0;
  virtual ~IMarketDataSink() = default;
};
