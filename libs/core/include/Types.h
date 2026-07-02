#pragma once

#include <cstdint>
#include <list>
#include <map>

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

// This use used so we can cache the total quantity available at a price level.
// This is requied to sent out level updates for each price
struct PriceLevel {
  Quantity totalQty = 0;
  OrderList orders;
};

using BookSide = std::map<Price, PriceLevel>;

enum class Side { Bid, Ask };
