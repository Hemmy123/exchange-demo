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

  // Note: One order can result in multiple events happening.
  auto internalEvents = book.DrainInteralEvents();

  for (const auto &event : internalEvents) {
    m_internalEventsSink.Publish(event);
  }
}

void MatchingEngine::Modify(InstrumentId instrument, OrderId id,
                            std::optional<Price> price,
                            std::optional<Quantity> qty) {

  if (m_books.contains(instrument) == false) {
    // TODO: Log error
    return;
  }

  m_books.at(instrument).Modify(id, price, qty);
}

void MatchingEngine::Delete(InstrumentId instrument, OrderId orderId) {
  if (m_books.contains(instrument) == false) {
    return;
  }

  m_books.at(instrument).Delete(orderId);
}
