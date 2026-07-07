#include "FeedMessage.h"    // FeedMessage { seq, timeStamp, payload }  (core)
#include "InternalEvents.h" // InternalEvent, LevelChangedEvent          (core)
#include "Sequencer.h"      // Sequencer                                 (feed)
#include "Types.h"          // SeqNum, Quantity
#include <gtest/gtest.h>
#include <vector>

// Seqnums start at 1 and rise by exactly 1 per event, no gaps -- the property
// downstream gap detection depends on.
TEST(Sequencer, AssignsGapFreeMonotonicSeqNums) {
  Sequencer sequencer;
  std::vector<SeqNum> seqs;

  for (Quantity q : {10, 15, 0}) {
    InternalEvent ev = LevelChangedEvent{
        .instrumentId = 1, .side = Side::Bid, .price = 100, .totalQty = q};
    seqs.push_back(sequencer.Sequence(ev).seq);
  }

  ASSERT_EQ(seqs.size(), 3u);
  EXPECT_EQ(seqs[0], static_cast<SeqNum>(1));
  EXPECT_EQ(seqs[1], static_cast<SeqNum>(2));
  EXPECT_EQ(seqs[2], static_cast<SeqNum>(3));
}
