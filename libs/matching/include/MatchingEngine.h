#pragma once

#include "IMarketDataSink.h"
#include "InternalEvents.h"
#include "Types.h"

#include <unordered_map>

class OrderBook;

class MatchingEngine {
public:
  explicit MatchingEngine(IMarketDataSink &sink) : m_marketDataSink(sink) {};

  // Note: OrderID is per instrument and not compaitable across instruments!
  // TODO: Order is mutated in this call. Should this be changed?
  std::vector<TradeEvent> PlaceOrder(InstrumentId instrument, Side side,
                                     Order &params);

  void Modify(InstrumentId instrument, OrderId id, std::optional<Price> price,
              std::optional<Quantity> qty);

  // Currently Modify loses the time priority for an order because it deletes
  // and re-inserts into the list. Most real exhanges allow for downward
  // quantity modification without losing priority (i.e. they can modify
  // quentity without being moved in the list) It might be worth making a
  // separate function for this
  //
  // void ModifyQuantity(InstrumentId instrument, OrderId orderId, Quantity
  // qty);

  void Delete(InstrumentId instrument, OrderId id);

private:
  std::vector<TradeEvent> StampAndCollect(OrderBook &book);

  std::unordered_map<InstrumentId, OrderBook> m_books;

  IMarketDataSink &m_marketDataSink;

  // 0 reserved for invalid
  TradeId m_nextTradeId{1};
};
