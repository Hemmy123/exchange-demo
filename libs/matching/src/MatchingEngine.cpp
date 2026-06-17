#include "MatchingEngine.h"
#include "OrderBook.h"

std::vector<TradeEvent> MatchingEngine::PlaceOrder(InstrumentId instrument,
                                                   Side side,
                                                   OrderParams params) {
  return {};
}

void MatchingEngine::Modify(InstrumentId instrument, OrderId id,
                            std::optional<Price> price,
                            std::optional<Quantity> qty) {}

void MatchingEngine::Delete(InstrumentId instrument, OrderId id) {}
