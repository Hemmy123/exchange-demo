#pragma once
#include "InternalEvents.h"

struct IMarketDataSink {
  virtual void Publish(const InternalEvent &ev) = 0;
  virtual ~IMarketDataSink() = default;
};
