#include "FeedMessage.h"    // FeedMessage { seq, timeStamp, payload }  (core)
#include "InternalEvents.h" // InternalEvent, LevelChangedEvent          (core)
#include "MarketDataEvents.h" // MarketDataEvent, MdLevelUpdate, IMarketDataTransport
#include "MarketDataPublisher.h"
#include "Types.h" // SeqNum, Side, Price, Quantity
#include <gtest/gtest.h>
#include <utility>
#include <variant>
#include <vector>

namespace {

// Captures every MarketDataEvent the publisher forwarded.
class CapturingTransport : public IMarketDataTransport {
public:
  void Send(const MarketDataEvent &ev) override { events.push_back(ev); }
  std::vector<MarketDataEvent> events;
};

// Frame an InternalEvent the way the publisher expects to receive it, without
// pulling in feed's Sequencer. The seq is ours to pick here: *generating* it is
// the Sequencer's job (tested in libs/feed); *propagating* it is the
// publisher's.
FeedMessage framed(SeqNum seq, InternalEvent payload) {
  return FeedMessage{
      .seq = seq, .timeStamp = {}, .payload = std::move(payload)};
}

} // namespace

TEST(MarketData, LevelChangedProjectsToMdLevelUpdate) {
  CapturingTransport transport;
  MarketDataPublisher publisher{transport};

  InternalEvent ev = LevelChangedEvent{
      .instrumentId = 1, .side = Side::Bid, .price = 100, .totalQty = 15};
  publisher.Publish(framed(1, ev));

  ASSERT_EQ(transport.events.size(), 1u);
  const auto *md = std::get_if<MdLevelUpdate>(&transport.events[0].body);
  ASSERT_NE(md, nullptr) << "body was not an MdLevelUpdate";
  EXPECT_EQ(md->side, Side::Bid);
  EXPECT_EQ(md->price, 100);
  EXPECT_EQ(md->totalQty, 15);
}

TEST(MarketData, LevelGoneProjectsZeroQty) {
  CapturingTransport transport;
  MarketDataPublisher publisher{transport};

  InternalEvent ev = LevelChangedEvent{
      .instrumentId = 1, .side = Side::Ask, .price = 101, .totalQty = 0};
  publisher.Publish(framed(1, ev));

  ASSERT_EQ(transport.events.size(), 1u);
  const auto *md = std::get_if<MdLevelUpdate>(&transport.events[0].body);
  ASSERT_NE(md, nullptr);
  EXPECT_EQ(md->totalQty, 0); // consumer erases the level on this
}

// The publisher forwards the seq it was handed, unchanged. Whether that seq is
// gap-free is the Sequencer's contract, not this one's.
TEST(MarketData, PublisherPreservesSeqNum) {
  CapturingTransport transport;
  MarketDataPublisher publisher{transport};

  InternalEvent ev = LevelChangedEvent{
      .instrumentId = 1, .side = Side::Bid, .price = 100, .totalQty = 10};
  publisher.Publish(framed(42, ev));

  ASSERT_EQ(transport.events.size(), 1u);
  EXPECT_EQ(transport.events[0].seqNum, static_cast<SeqNum>(42));
}
