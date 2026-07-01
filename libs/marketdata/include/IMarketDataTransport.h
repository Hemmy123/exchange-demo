#pragma once

#include "MarketDataEvents.h"

struct IMarketDataTransport {
  virtual void Send(const MarketDataEvent &ev) = 0;
  virtual ~IMarketDataTransport() = default;
};
