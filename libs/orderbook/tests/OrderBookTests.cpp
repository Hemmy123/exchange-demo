#include "OrderBook.h"
#include "Types.h"
#include <array>
#include <gtest/gtest.h>
#include <numeric>
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
  std::array<Order, 7> askOrders{
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
  std::array<Order, 7> bidOrders{
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
bool CompareBookMap(const BookSide &left, const BookSide &right) {

  for (const auto &elem : left) {
    auto leftPrice = elem.first;
    const auto &leftList = elem.second.orders;

    // check keys
    if (right.contains(leftPrice) == false)
      return false;

    // compare lists
    const auto &rightList = right.at(leftPrice).orders;
    if (leftList != rightList)
      return false;
  }

  return true;
}

template <typename T>
std::vector<T> filterEvents(const std::vector<InternalEvent> &evs) {
  std::vector<T> out;
  for (const auto &e : evs)
    if (const auto *p = std::get_if<T>(&e))
      out.push_back(*p);
  return out;
}

::testing::AssertionResult LevelEq(const LevelChangedEvent &e, Side side,
                                   Price price, Quantity qty) {
  if (e.side == side && e.price == price && e.totalQty == qty)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure()
         << "expected LevelChanged{side=" << static_cast<int>(side)
         << ", price=" << price << ", qty=" << qty
         << "} got {side=" << static_cast<int>(e.side) << ", price=" << e.price
         << ", qty=" << e.totalQty << "}";
}
} // namespace

struct OrderBookTestPeer {
  static auto &Orders(OrderBook &book) { return book.m_orders_map; }
  static auto &Bids(OrderBook &book) { return book.m_bidsMap; }
  static auto &Asks(OrderBook &book) { return book.m_askMap; }

  static void CheckLevelTotalsConsistent(const OrderBook &book) {
    auto checkSide = [&](const BookSide &side, const Side s) {
      for (const auto &[price, level] : side) {

        // Sum each level manually
        Quantity summed = std::accumulate(
            level.orders.cbegin(), level.orders.cend(), Quantity{0},
            [](Quantity acc, const Order &order) { return acc + order.qty; });

        // Check manually summed against the cached quantity
        EXPECT_EQ(level.totalQty, summed)
            << "cached total diverged at " << (s == Side::Bid ? "bid " : "ask ")
            << price;
      }
    };

    checkSide(book.m_bidsMap, Side::Bid);
    checkSide(book.m_askMap, Side::Ask);
  }
};

class OrderBookTestFixture : public testing::Test {
protected:
  OrderBook book{0};

  void SetUp() override { book = InitBook(); }

  void TearDown() override {
    // GoogleTest still runs TearDown even if a test fails with a fatal
    // assertion. If this happens we don't want to check for level consistency
    // because chances
    if (!HasFailure()) {
      OrderBookTestPeer::CheckLevelTotalsConsistent(book);
    }
  }
};
// --------------------------------------------------------------//

TEST_F(OrderBookTestFixture, EmptyBook) {
  EXPECT_FALSE(book.BestAsk().has_value());
  EXPECT_FALSE(book.BestBid().has_value());
  EXPECT_FALSE(book.Spread().has_value());
}

TEST_F(OrderBookTestFixture, BestAskFromList) {
  PlaceDefaultAskOrders(book);

  auto bestAsk = book.BestAsk();
  EXPECT_TRUE(bestAsk.has_value());
  EXPECT_EQ(bestAsk.value(), 101);
}

TEST_F(OrderBookTestFixture, BestBidFromList) {
  PlaceDefaultBidOrders(book);

  auto bestBid = book.BestBid();
  EXPECT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid.value(), 99);
}

TEST_F(OrderBookTestFixture, BestAskAndBidOrders) {
  // Testing if the BestBidOrder/BestAskOrder functions
  // return the same price as the BestBid/BestAsk functions
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

TEST_F(OrderBookTestFixture, BookSpread) {
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

TEST_F(OrderBookTestFixture, ModifyUnknownID) {
  book.PlaceOrder(Side::Bid, {.id = g_orderId, .price = 100, .qty = 10});

  // Modify unkonwn ID should do nothing;
  book.Modify(999, 200, std::nullopt);

  ASSERT_TRUE(book.BestBid().has_value());
  EXPECT_EQ(book.BestBid().value(), Price{100}); // still 100, not 200
  EXPECT_FALSE(book.BestAsk().has_value());      // nothing leaked onto asks
}

TEST_F(OrderBookTestFixture, ModifyOrderSameValuesNoOp) {
  // Testing the following from the modify function:
  // - Modifying the price/order with the same value shouldn't change anything
  // - Modifying the price/order with the nullopt values shouldn't change
  // anything
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

TEST_F(OrderBookTestFixture, QuantityAtPrice) {
  PlaceAllDefaultOrders(book);

  auto totalAskQty = book.QuantityAtPrice(Side::Ask, 102);
  auto totalBidQty = book.QuantityAtPrice(Side::Bid, 99);

  Quantity expectedAsk = 400 + 90 + 60;
  Quantity expectedBid = 300 + 145 + 80;

  EXPECT_EQ(totalAskQty, expectedAsk);
  EXPECT_EQ(totalBidQty, expectedBid);
}

// ===== Matching / Crossing Tests =====//
// An incoming Bid matches against the ask side (MatchAgainstAsks).
// These exercise the crossing path: sweeping a full level, partially
// filling the next resting order, and resting any leftover quantity.

TEST_F(OrderBookTestFixture, MatchBidCrossesAndClearsLevel) {
  PlaceAllDefaultOrders(book);

  // A Bid that only crosses the ask side must leave bids untouched.
  auto bidsBefore = OrderBookTestPeer::Bids(book);

  // best ask starts at 101
  EXPECT_EQ(book.BestAsk().value(), 101);

  // Incoming BUY: price 102, qty 500. Walks asks cheapest-first:
  //   101: 250 (id0) + 175 (id1) = 425 consumed -> level cleared
  //   102:  75 of the 400 (id2) consumed       -> id2 left with 325
  //   filled = 500 exactly, so nothing rests
  constexpr OrderId incomingId = 100;
  book.PlaceOrder(Side::Bid, {.id = incomingId, .price = 102, .qty = 500});

  // 101 fully consumed, best ask moves up to 102
  EXPECT_FALSE(OrderBookTestPeer::Asks(book).contains(101));
  EXPECT_EQ(book.BestAsk().value(), 102);

  // 102 level partially eaten
  Quantity expectedAsk102 = 325 + 90 + 60;
  EXPECT_EQ(book.QuantityAtPrice(Side::Ask, 102), expectedAsk102);

  // front of 102 is the partially-filled id2, now at qty 325
  auto bestAskOrder = book.BestAskOrder();
  ASSERT_TRUE(bestAskOrder.has_value());
  EXPECT_EQ(bestAskOrder.value().id, 2);
  EXPECT_EQ(bestAskOrder.value().qty, 325);

  // index stays in sync: consumed orders gone, survivor stays
  EXPECT_FALSE(book.Contains(0));
  EXPECT_FALSE(book.Contains(1));
  EXPECT_TRUE(book.Contains(2));

  // a fully-filled incoming order never rests
  EXPECT_FALSE(book.Contains(incomingId));

  // crossing the asks must not have touched the bid side
  auto bidsAfter = OrderBookTestPeer::Bids(book);
  EXPECT_TRUE(CompareBookMap(bidsBefore, bidsAfter));
}

TEST_F(OrderBookTestFixture, MatchBidCrossesWithRestingRemainder) {
  PlaceAllDefaultOrders(book);

  // Incoming BUY: price 103, qty 1000. Walks asks cheapest-first:
  //   101: 250 + 175       = 425 consumed -> level cleared
  //   102: 400 + 90 + 60   = 550 consumed -> level cleared
  //   104: 104 > 103, no longer crosses   -> stop
  //   filled = 975, remainder 25 rests as a new bid @ 103
  constexpr OrderId incomingId = 200;
  book.PlaceOrder(Side::Bid, {.id = incomingId, .price = 103, .qty = 1000});

  // two ask levels swept; 104 survives as the new best ask
  EXPECT_FALSE(OrderBookTestPeer::Asks(book).contains(101));
  EXPECT_FALSE(OrderBookTestPeer::Asks(book).contains(102));
  EXPECT_EQ(book.BestAsk().value(), 104);
  Quantity expectedAsk104 = 320 + 210;
  EXPECT_EQ(book.QuantityAtPrice(Side::Ask, 104), expectedAsk104);

  // the 25-lot remainder rested and is now the best bid
  EXPECT_EQ(book.BestBid().value(), 103);
  EXPECT_EQ(book.QuantityAtPrice(Side::Bid, 103), Quantity{25});
  EXPECT_TRUE(book.Contains(incomingId));

  // spread tightened to 104 - 103
  EXPECT_EQ(book.Spread().value(), 1);

  // index stays in sync across the swept levels
  EXPECT_FALSE(book.Contains(0));
  EXPECT_FALSE(book.Contains(4));
  EXPECT_TRUE(book.Contains(5));
}

TEST_F(OrderBookTestFixture, MatchAskCrossesAndClearsLevel) {
  PlaceAllDefaultOrders(book);

  // An Ask that only crosses the bid side must leave asks untouched.
  auto asksBefore = OrderBookTestPeer::Asks(book);

  // best bid starts at 99
  EXPECT_EQ(book.BestBid().value(), 99);

  // Incoming SELL: price 97, qty 600. Walks bids highest-first:
  //   99: 300 (id7) + 145 (id8) + 80 (id9) = 525 consumed -> level cleared
  //   97:  75 of the 480 (id10) consumed                  -> id10 left with 405
  //   filled = 600 exactly, so nothing rests
  constexpr OrderId incomingId = 100;
  book.PlaceOrder(Side::Ask, {.id = incomingId, .price = 97, .qty = 600});

  // 99 fully consumed, best bid moves down to 97
  EXPECT_FALSE(OrderBookTestPeer::Bids(book).contains(99));
  EXPECT_EQ(book.BestBid().value(), 97);

  // 97 level partially eaten
  Quantity expectedBid97 = 405 + 210;
  EXPECT_EQ(book.QuantityAtPrice(Side::Bid, 97), expectedBid97);

  // front of 97 is the partially-filled id10, now at qty 405
  auto bestBidOrder = book.BestBidOrder();
  ASSERT_TRUE(bestBidOrder.has_value());
  EXPECT_EQ(bestBidOrder.value().id, 10);
  EXPECT_EQ(bestBidOrder.value().qty, 405);

  // index stays in sync: consumed orders gone, survivor stays
  EXPECT_FALSE(book.Contains(7));
  EXPECT_FALSE(book.Contains(8));
  EXPECT_FALSE(book.Contains(9));
  EXPECT_TRUE(book.Contains(10));

  // a fully-filled incoming order never rests
  EXPECT_FALSE(book.Contains(incomingId));

  // crossing the bids must not have touched the ask side
  auto asksAfter = OrderBookTestPeer::Asks(book);
  EXPECT_TRUE(CompareBookMap(asksBefore, asksAfter));
}

TEST_F(OrderBookTestFixture, MatchAskCrossesWithRestingRemainder) {
  PlaceAllDefaultOrders(book);

  // Incoming SELL: price 96, qty 1300. Walks bids highest-first:
  //   99: 300 + 145 + 80   = 525 consumed -> level cleared
  //   97: 480 + 210        = 690 consumed -> level cleared
  //   95: 95 < 96, no longer crosses      -> stop
  //   filled = 1215, remainder 85 rests as a new ask @ 96
  constexpr OrderId incomingId = 200;
  book.PlaceOrder(Side::Ask, {.id = incomingId, .price = 96, .qty = 1300});

  // two bid levels swept; 95 survives as the new best bid
  EXPECT_FALSE(OrderBookTestPeer::Bids(book).contains(99));
  EXPECT_FALSE(OrderBookTestPeer::Bids(book).contains(97));
  EXPECT_EQ(book.BestBid().value(), 95);
  Quantity expectedBid95 = 75 + 260;
  EXPECT_EQ(book.QuantityAtPrice(Side::Bid, 95), expectedBid95);

  // the 85-lot remainder rested and is now the best ask
  EXPECT_EQ(book.BestAsk().value(), 96);
  EXPECT_EQ(book.QuantityAtPrice(Side::Ask, 96), Quantity{85});
  EXPECT_TRUE(book.Contains(incomingId));

  // spread tightened to 96 - 95
  EXPECT_EQ(book.Spread().value(), 1);

  // index stays in sync across the swept levels
  EXPECT_FALSE(book.Contains(7));
  EXPECT_FALSE(book.Contains(11));
  EXPECT_TRUE(book.Contains(12));
}

TEST_F(OrderBookTestFixture, DeleteLastBidAtLevelRemovesLevel) {
  // Two bid levels: 100 (one order) and 99 (one order)
  OrderId id100 = g_orderId++;
  OrderId id99 = g_orderId++;
  book.PlaceOrder(Side::Bid, {.id = id100, .price = 100, .qty = 50});
  book.PlaceOrder(Side::Bid, {.id = id99, .price = 99, .qty = 50});

  ASSERT_EQ(book.BestBid().value(), Price{100});

  book.Delete(id100);

  // Level 100 must be gone from the internal map
  EXPECT_FALSE(OrderBookTestPeer::Bids(book).contains(100));

  // BestBid must reflect the next level, not the now-empty one
  ASSERT_TRUE(book.BestBid().has_value());
  EXPECT_EQ(book.BestBid().value(), Price{99});
}

TEST_F(OrderBookTestFixture, DeleteLastAskAtLevelRemovesLevel) {
  // Two ask levels: 100 (one order) and 101 (one order)
  OrderId id100 = g_orderId++;
  OrderId id101 = g_orderId++;
  book.PlaceOrder(Side::Ask, {.id = id100, .price = 100, .qty = 50});
  book.PlaceOrder(Side::Ask, {.id = id101, .price = 101, .qty = 50});

  ASSERT_EQ(book.BestAsk().value(), Price{100});

  book.Delete(id100);

  // Level 100 must be gone from the internal map
  EXPECT_FALSE(OrderBookTestPeer::Asks(book).contains(100));

  // BestAsk must reflect the next level, not the now-empty one
  ASSERT_TRUE(book.BestAsk().has_value());
  EXPECT_EQ(book.BestAsk().value(), Price{101});
}

// TEST_F(OrderBookTestFixture, NormalUsageTest) { EXPECT_TRUE(false); }
TEST_F(OrderBookTestFixture, CachedTotalsSurviveCancels) {
  book.PlaceOrder(Side::Bid, {.id = 1, .price = 100, .qty = 10});
  book.PlaceOrder(Side::Bid, {.id = 2, .price = 100, .qty = 5});
  OrderBookTestPeer::CheckLevelTotalsConsistent(book); // Bid: 100: 10 -> 5

  book.Delete(1);
  OrderBookTestPeer::CheckLevelTotalsConsistent(book); // Bid: 100: 5

  book.PlaceOrder(Side::Ask,
                  {.id = 3, .price = 100, .qty = 12}); // crossing order
  OrderBookTestPeer::CheckLevelTotalsConsistent(book); // Ask: 100: 7
}

TEST(OrderBookLevels, FullTapeAcrossAddCancelCross) {
  OrderBook book{1};
  book.PlaceOrder(Side::Bid, {.id = 1, .price = 100, .qty = 10});
  book.PlaceOrder(Side::Bid, {.id = 2, .price = 100, .qty = 5});
  book.PlaceOrder(Side::Bid, {.id = 3, .price = 99, .qty = 8});
  book.PlaceOrder(Side::Ask, {.id = 4, .price = 101, .qty = 12});
  ASSERT_TRUE(book.Delete(2)); // cancel the 5 @ 100
  book.PlaceOrder(Side::Ask,
                  {.id = 5, .price = 100, .qty = 12}); // crosses bid 100

  auto levels = filterEvents<LevelChangedEvent>(book.DrainInteralEvents());
  ASSERT_EQ(levels.size(), 7u);
  EXPECT_TRUE(LevelEq(levels[0], Side::Bid, 100, 10));
  EXPECT_TRUE(LevelEq(levels[1], Side::Bid, 100, 15));
  EXPECT_TRUE(LevelEq(levels[2], Side::Bid, 99, 8));
  EXPECT_TRUE(LevelEq(levels[3], Side::Ask, 101, 12));
  EXPECT_TRUE(LevelEq(levels[4], Side::Bid, 100, 10)); // cancel: 15 -> 10
  EXPECT_TRUE(LevelEq(levels[5], Side::Bid, 100, 0));  // fill empties bid 100
  EXPECT_TRUE(LevelEq(levels[6], Side::Ask, 100, 2));  // remainder 2 rests
}
// Tests to write:
// Continous usage (read world usage):
//  - adding, modifying, deleting, checking spread
