#include "OrderBook.h"
#include "InternalEvents.h"
#include "Types.h"

#include <numeric>
#include <optional>
#include <print>
#include <vector>

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
  if (m_bidsMap.empty()) {
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

void OrderBook::PlaceOrder(const Side side, Order params) {

  if (side == Side::Ask) {
    // Bids match against opposite side (and vise versa)
    MatchAgainstBids(params);
  } else {
    MatchAgainstAsks(params);
  }

  // If there is still quantity left over, then add the rest to the book
  if (params.qty > 0) {
    auto &book = (side == Side::Ask) ? m_askMap : m_bidsMap;
    AddToSide(book, side, params);
  }
}

bool OrderBook::Modify(const OrderId id, std::optional<Price> newPrice,
                       std::optional<Quantity> newQty) {

  if (m_orders_map.contains(id) == false) {
    // TODO: Log warning here.
    return false;
  }

  // Remove and reinsert for new price
  const auto orderLocation = m_orders_map.at(id);

  // Doing some extra copies here that we could optimize out...
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
    return false;
  }

  // Erase the old order so we can read the new one.
  if (Delete(id) == false) {
    return false;
  }

  PlaceOrder(side, {.id = id, .price = price, .qty = qty});
  return true;
}

bool OrderBook::Delete(const OrderId id) {

  // make a separate DeleteInternal for both delete and Modify to use?
  if (m_orders_map.contains(id) == false) {
    return false;
  }

  const auto &orderLocation = m_orders_map.at(id);
  auto &list = orderLocation.levelIter->second;
  list.erase(orderLocation.orderIt);
  m_orders_map.erase(id);

  if (list.empty()) {
    auto &book = (orderLocation.side == Side::Bid) ? m_bidsMap : m_askMap;
    book.erase(orderLocation.levelIter);
  }

  OrderRemovedEvent orderRemovedEvent{.instrumentId = m_instrument,
                                      .orderId = id};

  m_internalEvents.push_back(orderRemovedEvent);

  return true;
}

std::optional<Quantity> OrderBook::QuantityAtPrice(Side side, Price price) {
  const auto &book = (side == Side::Ask) ? m_askMap : m_bidsMap;

  if (book.contains(price) == false) {
    return {};
  }

  const auto &priceList = book.at(price);

  return std::accumulate(
      priceList.cbegin(), priceList.cend(), Quantity{0},
      [](Quantity sum, const Order &order) { return sum + order.qty; });
}

bool OrderBook::Contains(OrderId orderId) const {
  // Note: We assume that m_order_maps will always be insync and contain
  // all the order IDs for both ask and buy books. If this goes out of
  // sync then we're in big trouble
  return m_orders_map.contains(orderId);
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

void OrderBook::MatchAgainstAsks(Order &incoming) {
  // the best ask price.
  // [Price -> OrderList]
  auto bestAskIter = m_askMap.begin();

  if (bestAskIter == m_askMap.end()) {
    return;
  }

  // clang-format off
  while (incoming.qty > 0 &&                        // has remaining orders
         bestAskIter != m_askMap.end() &&         // has price
         bestAskIter->first <= incoming.price) {  // is best price

    // clang-format on
    auto &ordersList = bestAskIter->second;

    FillLevel(Side::Bid, incoming, ordersList);

    // if we've exausted the orders in the list then we
    // need to remove the list
    if (ordersList.empty()) {
      bestAskIter = m_askMap.erase(bestAskIter);
    } else {
      break;
    }
  }
}

void OrderBook::MatchAgainstBids(Order &incoming) {

  // best bid is always the last one in the map so we only
  // need to check if the map is empty to work with it.
  while (incoming.qty && !m_bidsMap.empty()) {
    // Best bid is the _last_ (i.e. largest) bid.
    auto bestBidIter = std::prev(m_bidsMap.end());

    if (bestBidIter->first < incoming.price) {
      break;
    }

    FillLevel(Side::Ask, incoming, bestBidIter->second);

    if (bestBidIter->second.empty()) {
      m_bidsMap.erase(bestBidIter);
    } else {
      break;
    }
  }
}

void OrderBook::FillLevel(Side aggressorSide, Order &incoming,
                          OrderList &restingList) {
  while (incoming.qty > 0 && !restingList.empty()) {
    auto &oldestResting = restingList.front(); // FIFO: oldest first

    Quantity traded = std::min(incoming.qty, oldestResting.qty);
    incoming.qty -= traded;
    oldestResting.qty -= traded;

    // TODO: Placeholder logic for tracking what trades have happened
    // This will be replaced later.
    auto tradeEvent = TradeEvent{
        .tradeId = 0, // engine stamps later
        .instrumentId = m_instrument,
        .restingPrice = oldestResting.price,
        .quantityTraded = traded,
        .aggressorId = incoming.id,
        .restingId = oldestResting.id,
        .aggressorSide = aggressorSide,
        .restingRemaining = oldestResting.qty,
    };

    m_internalEvents.push_back(tradeEvent);

    if (oldestResting.qty == 0) {
      m_orders_map.erase(oldestResting.id); // keep the index consistent
      restingList.pop_front();
    }
  }
}

std::vector<InternalEvent> OrderBook::DrainInteralEvents() {
  auto out = std::move(m_internalEvents);
  m_internalEvents.clear();
  return out;
}
