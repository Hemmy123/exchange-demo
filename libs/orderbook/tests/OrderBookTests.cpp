#include "OrderBook.h"
#include "Types.h"
#include <array>
#include <gtest/gtest.h>
#include <optional>

// ------------------ Default dataset ---------------------------//

namespace {

OrderId g_orderId = 0;

// Creates a new book and resets global order id to 0
OrderBook InitBook() {
  constexpr InstrumentId instrument = 1;
  OrderBook newBook(instrument);
  g_orderId = 0;
  return newBook;
}

auto GetDefaultAskOrders() {
  std::array<OrderParams, 7> askOrders{
      {{.id = g_orderId++, .price = 101, .qty = 250},
       {.id = g_orderId++, .price = 101, .qty = 175},
       {.id = g_orderId++, .price = 102, .qty = 400},
       {.id = g_orderId++, .price = 102, .qty = 90},
       {.id = g_orderId++, .price = 102, .qty = 60},
       {.id = g_orderId++, .price = 104, .qty = 320},
       {.id = g_orderId++, .price = 104, .qty = 210}}};
  return askOrders;
}

auto GetDefaultBidOrders() {
  std::array<OrderParams, 7> bidOrders{
      {{.id = g_orderId++, .price = 99, .qty = 300},
       {.id = g_orderId++, .price = 99, .qty = 145},
       {.id = g_orderId++, .price = 99, .qty = 80},
       {.id = g_orderId++, .price = 97, .qty = 480},
       {.id = g_orderId++, .price = 97, .qty = 210},
       {.id = g_orderId++, .price = 95, .qty = 75},
       {.id = g_orderId++, .price = 95, .qty = 260}}};
  return bidOrders;
}

void PlaceDefaultAskOrders(OrderBook &book) {
  for (const auto &order : GetDefaultAskOrders()) {
    book.PlaceOrder(Side::Ask, order);
  }
}

void PlaceDefaultBidOrders(OrderBook &book) {
  for (const auto &order : GetDefaultBidOrders()) {
    book.PlaceOrder(Side::Bid, order);
  }
}

void PlaceAllDefaultOrders(OrderBook &book) {
  PlaceDefaultAskOrders(book);
  PlaceDefaultBidOrders(book);
}

// Compares elements of both maps. O(N)
bool CompareBookMap(const std::map<Price, OrderList> &left,
                    const std::map<Price, OrderList> &right) {

  for (const auto &elem : left) {
    auto leftPrice = elem.first;
    const auto &leftList = elem.second;

    // check keys
    if (right.contains(leftPrice) == false)
      return false;

    // compare lists
    const auto &rightList = right.at(leftPrice);
    if (leftList != rightList)
      return false;
  }

  return true;
}

} // namespace

struct OrderBookTestPeer {
  static auto &Orders(OrderBook &book) { return book.m_orders_map; }
  static auto &Bids(OrderBook &book) { return book.m_bidsMap; }
  static auto &Asks(OrderBook &book) { return book.m_askMap; }
};
// --------------------------------------------------------------//

TEST(OrderBook, EmptyBook) {
  auto book = InitBook();

  EXPECT_FALSE(book.BestAsk().has_value());
  EXPECT_FALSE(book.BestBid().has_value());
  EXPECT_FALSE(book.Spread().has_value());
}

TEST(OrderBook, BestAskFromList) {
  auto book = InitBook();
  PlaceDefaultAskOrders(book);

  auto bestAsk = book.BestAsk();
  EXPECT_TRUE(bestAsk.has_value());
  EXPECT_EQ(bestAsk.value(), 101);
}

TEST(OrderBook, BestBidFromList) {
  auto book = InitBook();
  PlaceDefaultBidOrders(book);

  auto bestBid = book.BestBid();
  EXPECT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid.value(), 99);
}

TEST(OrderBook, BestAskAndBidOrders) {
  // Testing if the BestBidOrder/BestAskOrder functions
  // return the same price as the BestBid/BestAsk functions
  auto book = InitBook();
  PlaceAllDefaultOrders(book);
  book.Print();
  auto bestAsk = book.BestAsk();
  auto bestBid = book.BestBid();

  auto bestAskOrder = book.BestAskOrder();
  auto bestBidOrder = book.BestBidOrder();

  EXPECT_TRUE(bestAsk.has_value());
  EXPECT_TRUE(bestBid.has_value());
  EXPECT_TRUE(bestAskOrder.has_value());
  EXPECT_TRUE(bestBidOrder.has_value());

  EXPECT_EQ(bestAsk.value(), bestAskOrder.value().price);
  EXPECT_EQ(bestBid.value(), bestBidOrder.value().price);
}

TEST(OrderBook, BookSpread) {
  auto book = InitBook();
  PlaceAllDefaultOrders(book);

  auto spread = book.Spread();
  EXPECT_TRUE(spread.has_value());
  // spread = diff(highest bid - lowest ask)
  // spread = 400 - 100
  EXPECT_EQ(spread.value(), 2);
}

// ===== Modify Function Tests =====//
// Modify function has a few branches so we need to
// check they all get executed

TEST(OrderBook, ModifyUnknownID) {
  auto book = InitBook();
  book.PlaceOrder(Side::Bid, {.id = g_orderId, .price = 100, .qty = 10});

  // Modify unkonwn ID should do nothing;
  book.Modify(999, 200, std::nullopt);

  ASSERT_TRUE(book.BestBid().has_value());
  EXPECT_EQ(book.BestBid().value(), Price{100}); // still 100, not 200
  EXPECT_FALSE(book.BestAsk().has_value());      // nothing leaked onto asks
}

TEST(OrderBook, ModifyOrderSameValuesNoOp) {
  // Testing the following from the modify function:
  // - Modifying the price/order with the same value shouldn't change anything
  // - Modifying the price/order with the nullopt values shouldn't change
  // anything
  auto book = InitBook();
  PlaceAllDefaultOrders(book);

  auto asksBefore = OrderBookTestPeer::Asks(book);
  auto bidsBefore = OrderBookTestPeer::Bids(book);

  g_orderId = 0;
  auto defaultAsk = GetDefaultAskOrders();
  auto defaultBids = GetDefaultBidOrders();

  // orders picked at random
  auto askOrder1 = defaultAsk[0];
  auto askOrder2 = defaultAsk[3];
  auto bidOrder1 = defaultBids[2];
  auto bidOrder2 = defaultBids[4];

  // same price, no qty change
  book.Modify(askOrder1.id, askOrder1.price, {});
  // no price, same qty
  book.Modify(askOrder2.id, {}, askOrder2.qty);

  // repeated but for bid side
  book.Modify(bidOrder1.id, bidOrder1.price, {});
  book.Modify(bidOrder2.id, {}, bidOrder2.qty);

  auto asksAfter = OrderBookTestPeer::Asks(book);
  auto bidsAfter = OrderBookTestPeer::Bids(book);

  EXPECT_TRUE(CompareBookMap(asksBefore, asksAfter));
  EXPECT_TRUE(CompareBookMap(bidsBefore, bidsAfter));
}

TEST(OrderBook, QuantityAtPrice) {
  auto book = InitBook();
  PlaceAllDefaultOrders(book);

  auto totalAskQty = book.QuantityAtPrice(Side::Ask, 102);
  auto totalBidQty = book.QuantityAtPrice(Side::Bid, 99);

  Quantity expectedAsk = 400 + 90 + 60;
  Quantity expectedBid = 300 + 145 + 80;

  EXPECT_EQ(totalAskQty, expectedAsk);
  EXPECT_EQ(totalBidQty, expectedBid);
}

// TEST(OrderBook, NormalUsageTest) { EXPECT_TRUE(false); }

// Tests to write:
// Continous usage (read world usage):
//  - adding, modifying, deleting, checking spread
