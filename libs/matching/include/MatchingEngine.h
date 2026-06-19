#pragma once

#include "IMarketDataSink.h"
#include "InternalEvents.h"
#include "Types.h"

#include <unordered_map>

class OrderBook;

class MatchingEngine {
public:
  explicit MatchingEngine(IMarketDataSink &sink) : m_marketDataSink(sink) {};

  std::vector<TradeEvent> PlaceOrder(InstrumentId instrument, Side side,
                                     Order &params);

  void Modify(InstrumentId instrument, OrderId id, std::optional<Price> price,
              std::optional<Quantity> qty);

  void Delete(InstrumentId instrument, OrderId id);

private:
  std::vector<TradeEvent> StampAndCollect(OrderBook &book);

  std::unordered_map<InstrumentId, OrderBook> m_books;

  IMarketDataSink &m_marketDataSink;

  // 0 reserved for invalid
  TradeId m_nextTradeId{1};
};
