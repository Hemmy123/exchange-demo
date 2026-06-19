#pragma once

#include "Types.h"
#include <chrono>
#include <variant>

// Used to broadcast a trade that has just happened
struct MdTrade {
  Price price;
  Quantity qty;
  Side agressorSide;
  TradeId tradeId;
};

// L2 Market Data:
// Provides updates on when price levels and quantities change.
// - One trade can triggle many level updates. for example, if a trade
//   buys out 5 price levels of trades then 5 of these level updates
//   would be sent out, showing they all have an new quantity of 0.
//   A 6th even might also be sent out of there are remaining
//   unfulfilled qty left over from the original trade request
struct MdLevelUpdate {
  Side side;
  Price price;
  Quantity totalQty;
};

struct MarketDataEvent {
  // Header
  SeqNum seqNum;
  InstrumentId instrumentId;
  std::chrono::system_clock::time_point timeStamp;

  // body
  std::variant<MdTrade, MdLevelUpdate> body;
};
