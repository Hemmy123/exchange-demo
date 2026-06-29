#include "InternalEvents.h"
#include "MarketDataEvents.h"
#include "MarketDataPublisher.h"
#include "Types.h"
#include <chrono>
#include <gtest/gtest.h>
#include <variant>
#include <vector>

namespace {

// Records what the publisher would have sent, so we can assert on it.
struct CollectingTransport : IMarketDataTransport {
  std::vector<MarketDataEvent> sent;
  void Send(const MarketDataEvent &ev) override { sent.push_back(ev); }
};

} // namespace

// --- Normalization --------------------------------------------------------//
//
// TEST(MarketDataPublisher, TradeNormalizedToMdTrade) {
//   CollectingTransport transport;
//   MarketDataPublisher publisher{transport};
//
//   TradeEvent trade{
//       .tradeId = 7,
//       .instrumentId = 7,
//       .restingPrice = 100,
//       .quantityTraded = 4,
//       .aggressorId = 20,
//       .restingId = 10,
//       .aggressorSide = Side::Bid,
//       .restingRemaining = 0,
//       .timeStamp = std::chrono::system_clock::now(),
//   };
//
//   publisher.Publish(trade);
//
//   ASSERT_EQ(transport.sent.size(), 1u);
//   const auto &md = transport.sent.front();
//
//   EXPECT_EQ(md.seqNum, 1u); // first published -> seq 1
//   EXPECT_EQ(md.instrumentId, 7u);
//   EXPECT_EQ(md.timeStamp, trade.timeStamp); // carried through, NOT
//   re-stamped
//
//   ASSERT_TRUE(std::holds_alternative<MdTrade>(md.body));
//   const auto &t = std::get<MdTrade>(md.body);
//   EXPECT_EQ(t.price, 100u);
//   EXPECT_EQ(t.qty, 4u);
//   EXPECT_EQ(t.agressorSide, Side::Bid);
//   EXPECT_EQ(t.tradeId, 7u);
//   // The public type structurally has no order ids — privacy is enforced by
//   // the type, not a runtime check.
// }
//
// // --- Sequencing
// -----------------------------------------------------------//
//
// TEST(MarketDataPublisher, SequenceNumbersIncrementPerPublishedEvent) {
//   CollectingTransport transport;
//   MarketDataPublisher publisher{transport};
//
//   publisher.Publish(TradeEvent{.tradeId = 1,
//                                .instrumentId = 1,
//                                .restingPrice = 100,
//                                .quantityTraded = 10,
//                                .aggressorId = 1,
//                                .restingId = 2,
//                                .aggressorSide = Side::Bid,
//                                .restingRemaining = 0,
//                                .timeStamp =
//                                std::chrono::system_clock::now()});
//   publisher.Publish(TradeEvent{.tradeId = 2,
//                                .instrumentId = 1,
//                                .restingPrice = 101,
//                                .quantityTraded = 5,
//                                .aggressorId = 3,
//                                .restingId = 4,
//                                .aggressorSide = Side::Bid,
//                                .restingRemaining = 0,
//                                .timeStamp =
//                                std::chrono::system_clock::now()});
//   publisher.Publish(TradeEvent{.tradeId = 3,
//                                .instrumentId = 1,
//                                .restingPrice = 102,
//                                .quantityTraded = 1,
//                                .aggressorId = 5,
//                                .restingId = 6,
//                                .aggressorSide = Side::Bid,
//                                .restingRemaining = 0,
//                                .timeStamp =
//                                std::chrono::system_clock::now()});
//
//   ASSERT_EQ(transport.sent.size(), 3u);
//   EXPECT_EQ(transport.sent[0].seqNum, 1u);
//   EXPECT_EQ(transport.sent[1].seqNum, 2u);
//   EXPECT_EQ(transport.sent[2].seqNum, 3u);
// }
//
// TEST(MarketDataPublisher, FilteredEventsDoNotConsumeSequenceNumbers) {
//   CollectingTransport transport;
//   MarketDataPublisher publisher{transport};
//
//   publisher.Publish(
//       TradeEvent{.tradeId = 1,
//                  .instrumentId = 1,
//                  .restingPrice = 100,
//                  .quantityTraded = 10,
//                  .aggressorId = 1,
//                  .restingId = 2,
//                  .aggressorSide = Side::Bid,
//                  .restingRemaining = 0,
//                  .timeStamp = std::chrono::system_clock::now()}); // -> seq 1
//   publisher.Publish(OrderAddedEvent{.instrumentId = 1,
//                                     .orderId = 3,
//                                     .side = Side::Ask,
//                                     .price = 105,
//                                     .qty = 50}); // not published
//   publisher.Publish(TradeEvent{
//       .tradeId = 2,
//       .instrumentId = 1,
//       .restingPrice = 100,
//       .quantityTraded = 5,
//       .aggressorId = 4,
//       .restingId = 5,
//       .aggressorSide = Side::Bid,
//       .restingRemaining = 0,
//       .timeStamp = std::chrono::system_clock::now()}); // -> seq 2, NOT 3
//
//   ASSERT_EQ(transport.sent.size(), 2u);
//   EXPECT_EQ(transport.sent[0].seqNum, 1u);
//   EXPECT_EQ(transport.sent[1].seqNum, 2u); // contiguous — no phantom gap
// }
//
// TEST(MarketDataPublisher, NonTradeEventPublishesNothingYet) {
//   CollectingTransport transport;
//   MarketDataPublisher publisher{transport};
//
//   publisher.Publish(OrderRemovedEvent{.instrumentId = 1, .orderId = 9});
//   EXPECT_TRUE(transport.sent.empty());
// }
//
// TEST(MarketDataPublisher, EachPublisherHasIndependentSequence) {
//   CollectingTransport tA, tB;
//   MarketDataPublisher a{tA};
//   MarketDataPublisher b{tB};
//
//   a.Publish(
//       TradeEvent{.tradeId = 1,
//                  .instrumentId = 1,
//                  .restingPrice = 100,
//                  .quantityTraded = 10,
//                  .aggressorId = 1,
//                  .restingId = 2,
//                  .aggressorSide = Side::Bid,
//                  .restingRemaining = 0,
//                  .timeStamp = std::chrono::system_clock::now()}); // a -> seq
//                  1
//   a.Publish(
//       TradeEvent{.tradeId = 2,
//                  .instrumentId = 1,
//                  .restingPrice = 100,
//                  .quantityTraded = 10,
//                  .aggressorId = 3,
//                  .restingId = 4,
//                  .aggressorSide = Side::Bid,
//                  .restingRemaining = 0,
//                  .timeStamp = std::chrono::system_clock::now()}); // a -> seq
//                  2
//   b.Publish(
//       TradeEvent{.tradeId = 3,
//                  .instrumentId = 1,
//                  .restingPrice = 100,
//                  .quantityTraded = 10,
//                  .aggressorId = 5,
//                  .restingId = 6,
//                  .aggressorSide = Side::Bid,
//                  .restingRemaining = 0,
//                  .timeStamp = std::chrono::system_clock::now()}); // b -> seq
//                  1
//
//   ASSERT_EQ(tA.sent.size(), 2u);
//   ASSERT_EQ(tB.sent.size(), 1u);
//   EXPECT_EQ(tA.sent[1].seqNum, 2u);
//   EXPECT_EQ(tB.sent[0].seqNum, 1u);
// }
