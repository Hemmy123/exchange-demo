#include "MarketDataPublisher.h"
#include "InternalEvents.h"
#include "MarketDataEvents.h"
#include <variant>

void MarketDataPublisher::Publish(const InternalEvent &ev) {

  if (auto *tradeEvent = std::get_if<TradeEvent>(&ev)) {
    MarketDataEvent md{
        .seqNum = m_nextSeq++,
        .instrumentId = tradeEvent->instrumentId,
        .timeStamp = tradeEvent->timeStamp,
        .body = MdTrade{tradeEvent->price, tradeEvent->quantityTraded,
                        tradeEvent->aggressorSide, tradeEvent->tradeId},
    };
    m_transport.Send(md);
    // serialize + send (UDP later)
  }

  // if(auto* orderAdded = std::get_if<OrderAddedEvent>(&ev) )
  // {
  //
  // }
  // if(auto* orderRemovedEvent = std::get_if<OrderRemovedEvent>(&ev))
  // {
  //
  // }
}
