#include "MatchingEngine.h"
#include "OrderBook.h"
#include "Types.h"

void MatchingEngine::PlaceOrder(InstrumentId instrument, Side side,
                                Order &params) {

  auto bookIter = m_books.find(instrument);

  if (bookIter == m_books.end()) {
    auto successCheck = m_books.insert({instrument, OrderBook(instrument)});
    if (successCheck.second == false) {
      // LOG ERROR HERE: Failed to insert into book
      return;
    }
    bookIter = successCheck.first;
  }

  auto &book = bookIter->second;
  book.PlaceOrder(side, params);
  DrainEvents(book);
}

void MatchingEngine::Modify(InstrumentId instrument, OrderId id,
                            std::optional<Price> price,
                            std::optional<Quantity> qty) {

  auto bookIter = m_books.find(instrument);

  if (bookIter == m_books.end()) {
    auto successCheck = m_books.insert({instrument, OrderBook(instrument)});
    if (successCheck.second == false) {
      // LOG ERROR HERE: Failed to insert into book
      return;
    }
    bookIter = successCheck.first;
  }

  auto &book = bookIter->second;
  book.Modify(id, price, qty);
  DrainEvents(book);
}

void MatchingEngine::Delete(InstrumentId instrument, OrderId orderId) {

  auto bookIter = m_books.find(instrument);
  if (bookIter == m_books.end()) {
    return;
  }

  auto &book = bookIter->second;
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
