#pragma once

#include "IInternalEventSink.h"
#include "OrderBook.h"
#include "Types.h"

#include <optional>
#include <unordered_map>

class MatchingEngine {
public:
  explicit MatchingEngine(IInteralEventSink &sink)
      : m_internalEventsSink(sink) {};

  // Note: OrderID is per instrument and not compaitable across instruments!
  // TODO: Order is mutated in this call. Should this be changed?
  void PlaceOrder(InstrumentId instrument, Side side, Order &params);

  void Modify(InstrumentId instrument, OrderId id, std::optional<Price> price,
              std::optional<Quantity> qty);

  // TODO:Currently Modify loses the time priority for an order because it
  // deletes and re-inserts into the list. Most real exhanges allow for downward
  // quantity modification without losing priority (i.e. they can modify
  // quentity without being moved in the list) It might be worth making a
  // separate function for this
  //
  // void ModifyQuantity(InstrumentId instrument, OrderId orderId, Quantity
  // qty);

  void Delete(InstrumentId instrument, OrderId id);

private:
  std::unordered_map<InstrumentId, OrderBook> m_books;

  IInteralEventSink &m_internalEventsSink;

  // 0 reserved for invalid
  TradeId m_nextTradeId{1};
};
