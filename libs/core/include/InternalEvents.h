#pragma once

#include "Types.h"
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

struct LevelChangedEvent {
  InstrumentId instrumentId;
  Side side;
  Price price;
  Quantity totalQty; // if == 0, then that means that level has been removed
};

// Note: A modify event is not needed because a modify is
// essentially a remove + add

using InternalEvent = std::variant<TradeEvent, OrderAddedEvent,
                                   OrderRemovedEvent, LevelChangedEvent>;
