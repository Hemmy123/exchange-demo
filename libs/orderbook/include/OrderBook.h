#pragma once

#include "Types.h"
#include "Utils.h"

#include <list>
#include <map>
#include <optional>
#include <unordered_map>

enum class Side { Bid, Ask };

struct OrderParams {
  OrderId id;
  Price price;
  Quantity qty;
};

// ----- Order Book ---- //
//

struct Order {
  OrderId id;
  Price price;
  Quantity qty;
};

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

  std::optional<Price> Spread() const;

  void Print() const;

private:
  using OrderList = std::list<Order>;

  InstrumentId m_instrument;

  // NOTE: Use reverse iterators with this! Not using std::greater<>
  // to keep the types of the 2 makes the same
  std::map<Price, OrderList> m_bidsMap;

  std::map<Price, OrderList> m_askMap;

  struct OrderLocation {
    Side side;
    std::map<Price, OrderList>::iterator levelIter;
    OrderList::iterator orderIt;
  };

  std::unordered_map<OrderId, OrderLocation> m_orders_map;

  template <typename BookSide>
  void AddToSide(BookSide &book, Side side, const OrderParams params);
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
