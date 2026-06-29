#pragma once

#include "Types.h"
#include <chrono>
#include <variant>

struct TradeEvent {
  TradeId tradeId;
  InstrumentId instrumentId;
  Price restingPrice;      // resting order price, NOT incoming price
  Quantity quantityTraded; // min(incoming,resting)
  OrderId aggressorId;     // ID of the incoming order
  OrderId restingId;       // ID of taken order, maker.
  Side aggressorSide;
  Quantity restingRemaining; //  if 0, order has been fulfilled and removed
};

struct OrderAddedEvent {
  InstrumentId instrumentId;
  OrderId orderId;
  Side side;
  Price price;
  Quantity qty;
};

struct OrderRemovedEvent {
  InstrumentId instrumentId;
  OrderId orderId;
};

using InternalEvent =
    std::variant<TradeEvent, OrderAddedEvent, OrderRemovedEvent>;

struct FeedMessage {
  std::uint64_t seq; // monotonic, for gap detection
  std::chrono::system_clock::time_point timeStamp; // shared a batch of events
  InternalEvent payload; // the variant: Trade / Added / Removed
};
// Note: A modify event is not needed because a modify is
// essentially a remove + add
//
