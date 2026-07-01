#include "MarketDataPublisher.h"
#include "FeedMessage.h"
#include "InternalEvents.h"
#include "MarketDataEvents.h"
#include <variant>

void MarketDataPublisher::Publish(const FeedMessage &ev) {

  MarketDataEvent marketDataEvent;
  marketDataEvent.seqNum = ev.seq;
  marketDataEvent.timeStamp = ev.timeStamp;

  if (const auto *tradeEvent = std::get_if<TradeEvent>(&ev.payload)) {
    marketDataEvent.instrumentId = tradeEvent->instrumentId;
    marketDataEvent.body = MdTrade{.price = tradeEvent->restingPrice,
                                   .qty = tradeEvent->quantityTraded,
                                   .agressorSide = tradeEvent->aggressorSide,
                                   .tradeId = tradeEvent->tradeId};
  };
  if (const auto *addedEvent = std::get_if<OrderAddedEvent>(&ev.payload)) {
    // marketDataEvent.instrumentad = addedEvent->instrumentId;
    // marketDataEvent.body = MdTrade{0, 0, Side::Ask, 0};
    // marketDataEvent.body = MdLevelUpdate{.side = addedEvent->side,
    //                                      .price = addedEvent->price,
    //                                      .totalQty = addedEvent->qty};
  }
  if (const auto *removedEvent = std::get_if<OrderRemovedEvent>(&ev.payload)) {

    // marketDataEvent.instrumentId = removedEvent->instrumentId;
    // marketDataEvent.body = MdTrade{0, 0, Side::Ask, 0};
    // marketDataEvent.body = MdLevelUpdate{.side = removedEvent->side,
    //                                      .price = removedEvent->price,
    //                                      .totalQty = 0};
    //
  }
  m_transport.Send(marketDataEvent);
}
//
// struct MdTrade {
//   Price price;
//   Quantity qty;
//   Side agressorSide;
//   TradeId tradeId;
// };
//
// struct MdLevelUpdate {
//   Side side;
//   Price price;
//   Quantity totalQty;
// };
