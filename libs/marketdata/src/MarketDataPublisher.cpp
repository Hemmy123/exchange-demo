#include "MarketDataPublisher.h"
#include "InternalEvents.h"
#include "MarketDataEvents.h"
#include <variant>

void MarketDataPublisher::Publish(const InternalEvent &ev) {
  if (const auto *tradeEvent = std::get_if<TradeEvent>(&ev)) {
    MarketDataEvent md{
        .seqNum = m_nextSeq++,
        .instrumentId = tradeEvent->instrumentId,
        .timeStamp = std::chrono::system_clock::now(),
        .body = MdTrade{tradeEvent->restingPrice, tradeEvent->quantityTraded,
                        tradeEvent->aggressorSide, tradeEvent->tradeId},
    };
    m_transport.Send(md);
  }
}
