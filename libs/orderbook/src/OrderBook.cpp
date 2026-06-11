#include "OrderBook.h"
#include "Types.h"

#include <optional>
#include <print>

namespace OrderBookHelpers {}

std::optional<Price> OrderBook::BestBid() const {
  if (m_bidsMap.empty()) {
    return {};
  }

  // highest price = best bid
  return m_bidsMap.rbegin()->first;
}

std::optional<Price> OrderBook::BestAsk() const {
  if (m_askMap.empty()) {
    return {};
  }

  // lowest price = best ask
  return m_askMap.begin()->first;
}

std::optional<Order> OrderBook::BestBidOrder() const {
  if (m_askMap.empty()) {
    return {};
  }

  return m_bidsMap.rbegin()->second.front();
}

std::optional<Order> OrderBook::BestAskOrder() const {
  if (m_askMap.empty()) {
    return {};
  }

  return m_askMap.begin()->second.front();
}

std::optional<Price> OrderBook::Spread() const {
  auto bestBid = BestBid();
  auto bestAsk = BestAsk();

  // a spread doesn't make sense if one side is empty.
  // We should only calculate it when there are both
  // buy and sell orders
  if (!bestBid || !bestAsk) {
    return {};
  }

  return abs(static_cast<int>(bestBid.value()) -
             static_cast<int>(bestAsk.value()));
}

void OrderBook::PlaceOrder(const Side side, OrderParams params) {
  auto &book = (side == Side::Ask) ? m_askMap : m_bidsMap;
  AddToSide(book, side, params);
}

void OrderBook::Modify(const OrderId id, std::optional<Price> newPrice,
                       std::optional<Quantity> newQty) {

  if (m_orders_map.contains(id) == false) {
    // TODO: Log warning here.
    return;
  }

  // Remove and reinsert for new price
  const auto &orderLocation = m_orders_map.at(id);
  const auto side = orderLocation.side;
  auto price = orderLocation.orderIt->price;
  auto qty = orderLocation.orderIt->qty;

  bool modified = false;

  if (newPrice.has_value() && *newPrice != price) {
    price = *newPrice;
    modified = true;
  }

  if (newQty.has_value() && *newQty != qty) {
    qty = *newQty;
    modified = true;
  }

  if (!modified) {
    // TODO: Add warning here
    return;
  }

  // Erase the old order so we can readd the new one.
  Delete(id);

  auto &book = (orderLocation.side == Side::Ask) ? m_askMap : m_bidsMap;
  AddToSide(book, side, {.id = id, .price = price, .qty = qty});
}

void OrderBook::Delete(const OrderId id) {

  // make a separate DeleteInternal for both delete and Modify to use?
  if (m_orders_map.contains(id) == false) {
    // TODO: Log warning here.
    return;
  }

  // Remove and reinsert for new price
  const auto &orderLocation = m_orders_map.at(id);
  auto &list = orderLocation.levelIter->second;
  list.erase(orderLocation.orderIt);
  m_orders_map.erase(id);
}

void OrderBook::Print() const {
  std::print("--- Bids ---\n");
  for (auto elem : m_bidsMap) {

    std::print("Price {}: ", elem.first);
    for (const auto &order : elem.second) {
      std::print("[ID: {}, Price {}, Quantity: {}], ", order.id, order.price,
                 order.qty);
    }
    std::print("\n");
  }

  std::print("--- Asks --- \n");
  for (auto elem : m_askMap) {

    std::print("Price {}: ", elem.first);
    for (const auto &order : elem.second) {
      std::print("[ID: {}, Price {}, Quantity: {}], ", order.id, order.price,
                 order.qty);
    }
    std::print("\n");
  }
}
