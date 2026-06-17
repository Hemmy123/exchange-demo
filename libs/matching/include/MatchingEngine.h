#pragma once

#include "Types.h"

#include <unordered_map>

class OrderBook;

class MatchingEngine {
public:
  std::vector<TradeEvent> PlaceOrder(InstrumentId instrument, Side side,
                                     OrderParams params);

  void Modify(InstrumentId instrument, OrderId id, std::optional<Price> price,
              std::optional<Quantity> qty);
  void Delete(InstrumentId instrument, OrderId id);

private:
  std::unordered_map<InstrumentId, OrderBook> m_books;
};
