#pragma once

#include "Types.h"

#include <concepts>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>

enum class Operation { Bid, Ask, Modify, Delete, Print };

// ------ Order Book Operations ----- //
//
struct OrderBookOperations {};

template <typename T>
concept OrderBookParams = std::derived_from<T, OrderBookOperations>;

struct AskOrderParams : OrderBookOperations {
  OrderId id;
  Price price;
  Quantity qty;
};

struct BidOrderParams : OrderBookOperations {
  OrderId id;
  Price price;
  Quantity qty;
};

struct ModifyParams : OrderBookOperations {
  OrderId id;

  // null price/quantity will result in no change
  std::optional<Price> newPrice;
  std::optional<Quantity> newQuantity;
};

struct DeleteParam : OrderBookOperations {
  OrderId id;
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
  OrderBook(const OrderBook &) = default;
  OrderBook(OrderBook &&) = default;
  OrderBook &operator=(const OrderBook &) = default;
  OrderBook &operator=(OrderBook &&) = default;
  OrderBook(InstrumentId instrumentId) : m_instrument(instrumentId) {};

  template <Operation op, typename Params>
  void ProcessOperation(Params &&params) {
    if constexpr (op == Operation::Bid) {
      return Bid(params);
    } else if constexpr (op == Operation::Ask) {
      return Ask(params);
    } else if constexpr (op == Operation::Modify) {
      return Modify(params);
    } else if constexpr (op == Operation::Delete) {
      return Delete(params);
    } else if constexpr (op == Operation::Print) {
    } else {
      static_assert(false, "Unhandle Orderbook Operation");
    }
  }

  // Returns the bid at the highest price
  std::optional<Price> BestBid() const;

  // Returns the bid at the lowest price
  std::optional<Price> BestAsk() const;

  std::optional<int> Spread() const;

  void Print();

private:
  void Ask(const AskOrderParams &params);
  void Bid(const BidOrderParams &params);
  void Modify(const ModifyParams &params);
  void Delete(const DeleteParam &params);

  InstrumentId m_instrument;

  OrderId m_idCounter;

  using OrderList = std::list<Order>;

  std::map<Price, OrderList, std::greater<>> m_bidsMap;

  std::map<Price, OrderList> m_askMap;

  std::unordered_map<OrderId, OrderList::iterator> m_order_map;
};
