#pragma once

#include "Types.h"
#include "Utils.h"

#include <list>
#include <map>
#include <optional>
#include <unordered_map>

struct OrderParams {
  OrderId id;
  Price price;
  Quantity qty;
};

// ----- Order Book ---- //

struct Order {
  OrderId id;
  Price price;
  Quantity qty;

  bool operator==(const Order &) const = default;
};

using OrderList = std::list<Order>;

class OrderBook {
public:
  OrderBook(InstrumentId instrumentId) : m_instrument(instrumentId) {};

  MOVE_ONLY(OrderBook)

  void Modify(const OrderId id, std::optional<Price> newPrice,
              std::optional<Quantity> newQuantity);

  void Delete(const OrderId id);

  void PlaceOrder(const Side side, OrderParams params);

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

private:
  InstrumentId m_instrument;

  std::map<Price, OrderList> m_bidsMap;

  std::map<Price, OrderList> m_askMap;

  struct OrderLocation {
    Side side;
    std::map<Price, OrderList>::iterator levelIter;
    OrderList::iterator orderIt;
  };

  std::unordered_map<OrderId, OrderLocation> m_orders_map;

  // TODO: Placeholder vector to keep track of what trades have
  // happened. This will be replaced later
  std::vector<TradeEvent> m_tradeEvents;

  template <typename BookSide>
  void AddToSide(BookSide &book, Side side, const OrderParams params);

  // We always want to match the incoming order with the
  // opposite side. So ask->bid and bid->ask
  void MatchAgainstAsks(OrderParams &incoming);

  void MatchAgainstBids(OrderParams &incoming);

  void FillLevel(Side aggressorside, OrderParams &incoming,
                 OrderList &restingList);

  // To enable whitebox testing of order book.
  friend struct OrderBookTestPeer;
};

// ====== template definitions ===== //

template <typename BookSide>
void OrderBook::AddToSide(BookSide &book, Side side, const OrderParams params) {
  auto priceLevelIter = book.try_emplace(params.price).first;
  auto &priceList = priceLevelIter->second;
  priceList.emplace_back(params.id, params.price, params.qty);

  m_orders_map[params.id] =
      OrderLocation{.side = side,
                    .levelIter = priceLevelIter,
                    .orderIt = std::prev(priceList.end())};
}
