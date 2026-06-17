#pragma once

#include <chrono>
#include <cstdint>
#include <list>

using OrderId = uint64_t;
using TradeId = uint64_t;

using Price = uint64_t;
using Quantity = uint64_t;
using InstrumentId = uint64_t;

struct OrderParams {
  OrderId id;
  Price price;
  Quantity qty;
};

struct Order {
  OrderId id;
  Price price;
  Quantity qty;

  bool operator==(const Order &) const = default;
};

using OrderList = std::list<Order>;

enum class Side { Bid, Ask };

struct TradeEvent {
  TradeId tradeId;
  InstrumentId instrumentId;
  Price price;             // resting order price, NOT incoming price
  Quantity quantityTraded; // min(incoming,resting)
  OrderId aggressorId;     // ID of the incoming order
  OrderId restingId;       // ID of taken order, maker.
  Side aggressorSide;
  std::chrono::system_clock::time_point timeStamp;
  Quantity restingRemaining; //  if 0, order has been fulfilled and removed
};
