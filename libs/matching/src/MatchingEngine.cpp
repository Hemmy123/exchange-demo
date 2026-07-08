#include "MatchingEngine.h"
#include "OrderBook.h"
#include "Types.h"

void MatchingEngine::PlaceOrder(InstrumentId instrument, Side side,
                                Order &params) {

  if (m_books.contains(instrument) == false) {
    m_books.insert({instrument, OrderBook(instrument)});
  }

  auto &book = m_books.at(instrument);
  book.PlaceOrder(side, params);
  DrainEvents(book);
}

void MatchingEngine::Modify(InstrumentId instrument, OrderId id,
                            std::optional<Price> price,
                            std::optional<Quantity> qty) {

  if (m_books.contains(instrument) == false) {
    // TODO: Log error
    return;
  }

  auto &book = m_books.at(instrument);
  book.Modify(id, price, qty);
  DrainEvents(book);
}

void MatchingEngine::Delete(InstrumentId instrument, OrderId orderId) {
  if (m_books.contains(instrument) == false) {
    return;
  }

  auto &book = m_books.at(instrument);
  book.Delete(orderId);
  DrainEvents(book);
}

void MatchingEngine::DrainEvents(OrderBook &book) {
  // Note: One order can result in multiple events happening.
  auto internalEvents = book.DrainInteralEvents();

  for (auto &event : internalEvents) {
    if (auto *tradeEvent = std::get_if<TradeEvent>(&event)) {
      tradeEvent->tradeId = m_nextTradeId++;
    }
    m_internalEventsSink.Publish(event);
  }
}
