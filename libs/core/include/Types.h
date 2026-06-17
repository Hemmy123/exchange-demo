#pragma once

#include <chrono>
#include <cstdint>

using OrderId = uint64_t;
using TradeId = uint64_t;

using Price = uint64_t;
using Quantity = uint64_t;
using InstrumentId = uint64_t;

enum class Side { Bid, Ask };

struct TradeEvent {
  TradeId tradeId;
  InstrumentId instrumentId;
  Price price;             // resting order price, NOT incoming price
  Quantity quantityTraded; // min(incoming,resting)
  OrderId aggressorId;     // ID of the incoming order
  OrderId restingId;       // ID of taken order, maker.
  Side aggressorSide;
  std::chrono::steady_clock timeStamp;
  Quantity restingRemaining; //  if 0, order has been fulfilled and removed
};
