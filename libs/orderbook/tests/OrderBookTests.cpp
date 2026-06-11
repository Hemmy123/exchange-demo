#include "OrderBook.h"
#include <array>
#include <gtest/gtest.h>

// ------------------ Default dataset ---------------------------//

OrderId orderId = 0;

auto getDefaultAskOrders() {
  std::array<OrderParams, 6> askOrders{
      {{.id = orderId++, .price = 100, .qty = 532},
       {.id = orderId++, .price = 100, .qty = 431},
       {.id = orderId++, .price = 300, .qty = 312},
       {.id = orderId++, .price = 300, .qty = 130},
       {.id = orderId++, .price = 400, .qty = 432},
       {.id = orderId++, .price = 400, .qty = 230}}};
  return askOrders;
}

auto getDefaultBidOrders() {
  std::array<OrderParams, 6> bidOrders{
      {{.id = orderId++, .price = 100, .qty = 100},
       {.id = orderId++, .price = 200, .qty = 320},
       {.id = orderId++, .price = 200, .qty = 102},
       {.id = orderId++, .price = 400, .qty = 132},
       {.id = orderId++, .price = 400, .qty = 134},
       {.id = orderId++, .price = 100, .qty = 243}}};
  return bidOrders;
}

// --------------------------------------------------------------//

TEST(OrderBook, EmptyBook) {
  OrderBook book(1);
  orderId = 1;

  EXPECT_FALSE(book.BestAsk().has_value());
  EXPECT_FALSE(book.BestBid().has_value());
  EXPECT_FALSE(book.Spread().has_value());
}

TEST(OrderBook, BestAskFromList) {
  OrderBook book(1);
  orderId = 1;

  for (const auto &order : getDefaultAskOrders()) {
    book.PlaceOrder(Side::Ask, order);
  }

  auto bestAsk = book.BestAsk();
  EXPECT_TRUE(bestAsk.has_value());
  EXPECT_EQ(bestAsk.value(), 100);
}

TEST(OrderBook, BestBidFromList) {
  OrderBook book(1);
  orderId = 1;

  for (const auto &order : getDefaultBidOrders()) {
    book.PlaceOrder(Side::Bid, order);
  }

  auto bestBid = book.BestBid();
  EXPECT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid.value(), 400);
}

TEST(OrderBook, BestAskAndBidOrders) {
  // Testing if the BestBidOrder/BestAskOrder functions
  // return the same price as the BestBid/BestAsk functions

  OrderBook book(1);
  orderId = 1;

  for (const auto &order : getDefaultBidOrders()) {
    book.PlaceOrder(Side::Bid, order);
  }

  for (const auto &order : getDefaultAskOrders()) {
    book.PlaceOrder(Side::Ask, order);
  }

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
  OrderBook book(1);
  orderId = 1;

  for (const auto &order : getDefaultAskOrders()) {
    book.PlaceOrder(Side::Ask, order);
  }

  for (const auto &order : getDefaultBidOrders()) {
    book.PlaceOrder(Side::Bid, order);
  }

  auto spread = book.Spread();
  EXPECT_TRUE(spread.has_value());
  // spread = diff(highest bid - lowest ask)
  // spread = 400 - 100
  EXPECT_EQ(spread.value(), 300);
}

// TEST(OrderBook, ModifyOrderInBook) { EXPECT_TRUE(false) }

// TEST(OrderBook, NormalUsageTest) { EXPECT_TRUE(false); }

// Tests to write:
// Modify
// Delete
// Continous usage (read world usage):
//  - adding, modifying, deleting, checking spread
