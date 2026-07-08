#pragma once

#include "InternalEvents.h"
#include "Types.h"
#include "Utils.h"

#include <cstdint>
#include <execution>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

class TradeEvent;

class OrderBook {
public:
  OrderBook(InstrumentId instrumentId) : m_instrument(instrumentId) {};

  OrderBook() = delete;

  MOVE_ONLY(OrderBook)

  bool Modify(const OrderId id, std::optional<Price> newPrice,
              std::optional<Quantity> newQuantity);

  bool Delete(const OrderId id);

  void PlaceOrder(const Side side, Order params);

  // Returns the bid at the highest price
  std::optional<Price> BestBid() const;

  // Returns the ask at the lowest price
  std::optional<Price> BestAsk() const;

  // Returns the order at the highest price
  // that was made earliest in the queue
  std::optional<Order> BestBidOrder() const;

  // Returns the order with the lowest price
  // that was made earliest in the queue
  std::optional<Order> BestAskOrder() const;

  // Sums all the prices for a particular level
  std::optional<Quantity> QuantityAtPrice(Side side, Price price);

  // Checks if the order exists in either ask or bid side.
  bool Contains(OrderId) const;

  std::optional<Price> Spread() const;

  void Print() const;

  std::vector<InternalEvent> DrainInteralEvents();

private:
  InstrumentId m_instrument;

  BookSide m_bidsMap;
  BookSide m_askMap;

  struct OrderLocation {
    Side side;
    BookSide::iterator levelIter;
    OrderList::iterator orderIt;
  };

  std::unordered_map<OrderId, OrderLocation> m_orders_map;

  // TODO: Placeholder vector to keep track of what trades have
  // happened. This will be replaced later
  std::vector<InternalEvent> m_internalEvents;

  template <typename BookSide>
  void AddToSide(BookSide &book, Side side, const Order params);

  // We always want to match the incoming order with the
  // opposite side. So ask->bid and bid->ask
  void MatchAgainstAsks(Order &incoming);

  void MatchAgainstBids(Order &incoming);

  void FillLevel(Side aggressorside, Order &incoming, PriceLevel &priceList);

  void AdjustLevel(Side side, Price price, PriceLevel &level,
                   std::int64_t delta);

  // To enable whitebox testing of order book.
  friend struct OrderBookTestPeer;
};

// ====== template definitions ===== //

template <typename Book>
void OrderBook::AddToSide(Book &book, Side side, const Order params) {

  auto priceLevelIter = book.try_emplace(params.price).first;
  auto &level = priceLevelIter->second;
  auto &priceList = level.orders;

  priceList.emplace_back(params.id, params.price, params.qty);

  m_orders_map[params.id] =
      OrderLocation{.side = side,
                    .levelIter = priceLevelIter,
                    .orderIt = std::prev(priceList.end())};

  // Updates the quantity for a given price and book side. This encapsulated the
  // change so we can also update the cached totalQty value and send out an
  // event of the change happening. ALL changes to price levels should go
  // through this function.
  AdjustLevel(side, params.price, level, params.qty);

  OrderAddedEvent addedEvent{.instrumentId = m_instrument,
                             .orderId = params.id,
                             .side = side,
                             .price = params.price,
                             .qty = params.qty};

  m_internalEvents.push_back(addedEvent);
}
