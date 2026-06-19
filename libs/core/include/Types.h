#pragma once

#include <cstdint>
#include <list>

using OrderId = uint64_t;
using TradeId = uint64_t;
using SeqNum = uint64_t;

using Price = uint64_t;
using Quantity = uint64_t;
using InstrumentId = uint64_t;

struct Order {
  OrderId id;
  Price price;
  Quantity qty;

  bool operator==(const Order &) const = default;
};

using OrderList = std::list<Order>;

enum class Side { Bid, Ask };
