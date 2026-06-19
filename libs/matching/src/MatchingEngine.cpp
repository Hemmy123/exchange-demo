#include "MatchingEngine.h"
#include "OrderBook.h"
#include "Types.h"
#include <chrono>

std::vector<TradeEvent> MatchingEngine::PlaceOrder(InstrumentId instrument,
                                                   Side side, Order &params) {

  if (m_books.contains(instrument) == false) {
    m_books.insert({instrument, OrderBook(instrument)});
  }

  auto &book = m_books.at(instrument);
  book.PlaceOrder(side, params);

  auto trades = StampAndCollect(book);

  for (const auto &trade : trades) {
    m_marketDataSink.Publish(trade);
  }

  return trades;
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

std::vector<TradeEvent> MatchingEngine::StampAndCollect(OrderBook &book) {
  auto trades = book.DrainTrades();

  for (auto &trade : trades) {
    trade.tradeId = m_nextTradeId++;
    trade.timeStamp = std::chrono::system_clock::now();
  }
  return trades;
}
