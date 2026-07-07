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
  if (const auto *levelEvent = std::get_if<LevelChangedEvent>(&ev.payload)) {
    marketDataEvent.instrumentId = levelEvent->instrumentId;
    marketDataEvent.body = MdLevelUpdate{.side = levelEvent->side,
                                         .price = levelEvent->price,
                                         .totalQty = levelEvent->totalQty};
  }
  m_transport.Send(marketDataEvent);
}
