#include "OrderBook.h"

#include <optional>
#include <print>

namespace OrderBookHelpers {}

std::optional<Price> OrderBook::BestBid() const {
  if (m_bidsMap.empty()) {
    return {};
  }

  return m_bidsMap.begin()->first;
}

std::optional<Price> OrderBook::BestAsk() const {
  if (m_askMap.empty()) {
    return {};
  }

  // return m_askMap.end()->first;
  return m_askMap.begin()->first;
}

std::optional<int> OrderBook::Spread() const {
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

void OrderBook::Ask(const AskOrderParams &params) {

  // if a price already exists at this price, we want to
  // add it to the end of the list. Orders at the same price
  // should priortize earlier orders first.
  if (m_askMap.contains(params.price)) {
    auto &orderList = m_askMap.at(params.price);
    orderList.push_back({params.id, params.price, params.qty});
    return;
  }

  Order newAsk = {.id = params.id, .price = params.price, .qty = params.qty};
  m_askMap[params.price] = OrderList{newAsk};
}

void OrderBook::Bid(const BidOrderParams &params) {

  // if a price already exists at this price, we want to
  // add it to the end of the list. Orders at the same price
  // should priortize earlier orders first.
  if (m_bidsMap.contains(params.price)) {
    auto &orderList = m_bidsMap.at(params.price);
    orderList.push_back({params.id, params.price, params.qty});
    return;
  }

  Order newBid = {.id = params.id, .price = params.price, .qty = params.qty};
  m_bidsMap[params.price] = OrderList{newBid};
}

void OrderBook::Print() {
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
