#include "IInternalEventSink.h"
#include "MatchingEngine.h"
#include "OrderBook.h" // OrderParams
#include "Types.h"     // Side, TradeEvent

#include <gtest/gtest.h>
#include <optional>

namespace {

// PlaceOrder takes Order by non-const ref, so it can't bind a temporary.
// This wrapper takes by value -> gives us a named lvalue to pass through.
void Place(MatchingEngine &engine, InstrumentId instrument, Side side,
           Order params) {
  engine.PlaceOrder(instrument, side, params);
}

// MatchingEngine requires an IEventSink to be passed in.
// We don't need to test publishing logic here so we just pass in a dummy.
class MarketSinkDummy : public IInteralEventSink {
public:
  void Publish(const InternalEvent &ev) override {
    m_internalEvents.push_back(ev);
  };

  std::vector<InternalEvent> m_internalEvents;
};

// Returns only the TradeEvents from the sink's collected events.
std::vector<TradeEvent> FilterTrades(const std::vector<InternalEvent> &events) {
  std::vector<TradeEvent> trades;
  for (const auto &ev : events) {
    if (const auto *te = std::get_if<TradeEvent>(&ev)) {
      trades.push_back(*te);
    }
  }
  return trades;
}

} // namespace

// --- Lifecycle ------------------------------------------------------------//

TEST(MatchingEngine, PlaceOnNewInstrumentCreatesBookAndRests) {

  MarketSinkDummy dummySink;
  MatchingEngine engine(dummySink);
  constexpr InstrumentId instrument = 42;
  // instrument 42 has never been seen: the engine must create the book lazily,
  // rest the order, and publish no trades (nothing to cross with).
  Place(engine, instrument, Side::Ask, {.id = 1, .price = 100, .qty = 10});

  // A single  add should trigger 2 events, added and level changed, which we
  // check for explicitly below
  ASSERT_EQ(dummySink.m_internalEvents.size(), 2u);

  EXPECT_TRUE(std::holds_alternative<LevelChangedEvent>(
      dummySink.m_internalEvents.front()));

  EXPECT_TRUE(std::holds_alternative<OrderAddedEvent>(
      dummySink.m_internalEvents.back()));
}

// --- Identity stamping ----------------------------------------------------//

TEST(MatchingEngine, CrossingOrderReturnsFullyStampedTrade) {
  MarketSinkDummy dummySink;
  MatchingEngine engine(dummySink);
  constexpr InstrumentId instrument = 7;

  // resting ask of 10 @ 100
  Place(engine, instrument, Side::Ask, {.id = 10, .price = 100, .qty = 10});

  // incoming bid of 4 @ 100 crosses; partially fills the resting ask
  Place(engine, instrument, Side::Bid, {.id = 20, .price = 100, .qty = 4});

  auto trades = FilterTrades(dummySink.m_internalEvents);

  ASSERT_EQ(trades.size(), 1u);
  const auto &t = trades.front();

  EXPECT_EQ(t.tradeId, 1u);
  EXPECT_EQ(t.instrumentId, instrument);
  EXPECT_EQ(t.restingPrice, 100u);
  EXPECT_EQ(t.quantityTraded, 4u);
  EXPECT_EQ(t.aggressorId, 20u);
  EXPECT_EQ(t.restingId, 10u);
  EXPECT_EQ(t.aggressorSide, Side::Bid);
  EXPECT_EQ(t.restingRemaining, 6u); // 10 - 4
}

TEST(MatchingEngine, TradeIdsAreMonotonicAcrossInstruments) {
  MarketSinkDummy dummySink;
  MatchingEngine engine(dummySink);

  // instrument 1: rest then cross -> trade id 1
  Place(engine, 1, Side::Ask, {.id = 1, .price = 100, .qty = 10});
  Place(engine, 1, Side::Bid, {.id = 2, .price = 100, .qty = 10});

  // instrument 2: rest then cross -> trade id 2, from the SAME engine counter
  Place(engine, 2, Side::Ask, {.id = 3, .price = 200, .qty = 5});
  Place(engine, 2, Side::Bid, {.id = 4, .price = 200, .qty = 5});

  auto trades = FilterTrades(dummySink.m_internalEvents);

  ASSERT_EQ(trades.size(), 2u);
  EXPECT_EQ(trades[0].tradeId, 1u);
  EXPECT_EQ(trades[0].instrumentId, 1u);
  EXPECT_EQ(trades[1].tradeId, 2u); // not 1 — the counter is global
  EXPECT_EQ(trades[1].instrumentId, 2u);
}

TEST(MatchingEngine, SweepProducesSequentialIdsWithinOneCall) {
  MarketSinkDummy dummySink;
  MatchingEngine engine(dummySink);

  // two resting asks at different levels
  Place(engine, 1, Side::Ask, {.id = 1, .price = 100, .qty = 10});
  Place(engine, 1, Side::Ask, {.id = 2, .price = 101, .qty = 10});

  // one bid sweeps both levels -> two trades, ids assigned in fill order
  Place(engine, 1, Side::Bid, {.id = 3, .price = 101, .qty = 20});

  auto trades = FilterTrades(dummySink.m_internalEvents);

  ASSERT_EQ(trades.size(), 2u);
  EXPECT_EQ(trades[0].tradeId, 1u);
  EXPECT_EQ(trades[1].tradeId, 2u);
  EXPECT_EQ(trades[0].restingPrice, 100u); // cheapest ask filled first
  EXPECT_EQ(trades[1].restingPrice, 101u);
}

// --- Routing / isolation --------------------------------------------------//

TEST(MatchingEngine, BooksAreIsolatedByInstrument) {
  MarketSinkDummy dummySink;
  MatchingEngine engine(dummySink);

  // rest an ask on instrument 2
  Place(engine, 2, Side::Ask, {.id = 1, .price = 100, .qty = 10});

  // a bid on instrument 1 at a price that WOULD cross if the books were shared
  Place(engine, 1, Side::Bid, {.id = 2, .price = 100, .qty = 10});

  // separate books -> nothing to match against -> it just rests, no trade
  EXPECT_TRUE(FilterTrades(dummySink.m_internalEvents).empty());
}

TEST(MatchingEngine, DeleteRoutesToCorrectBook) {
  MarketSinkDummy dummySink;
  MatchingEngine engine(dummySink);

  Place(engine, 1, Side::Ask, {.id = 1, .price = 100, .qty = 10}); // rests
  engine.Delete(1, 1);                                             // remove it

  // a crossing bid now finds an empty book -> no trade
  Place(engine, 1, Side::Bid, {.id = 2, .price = 100, .qty = 10});
  EXPECT_TRUE(FilterTrades(dummySink.m_internalEvents).empty());
}

// --- Unknown-instrument guards --------------------------------------------//

TEST(MatchingEngine, ModifyAndDeleteUnknownInstrumentAreNoOps) {
  MarketSinkDummy dummySink;
  MatchingEngine engine(dummySink);
  // no book for 999 exists; the contains-guards must make these safe no-ops,
  // not throw (which an unguarded m_books.at(...) would).
  EXPECT_NO_THROW(engine.Modify(999, 1, Price{100}, std::nullopt));
  EXPECT_NO_THROW(engine.Delete(999, 1));
}
