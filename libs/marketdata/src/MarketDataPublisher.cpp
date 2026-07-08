#include "MarketDataPublisher.h"
#include "FeedMessage.h"
#include "InternalEvents.h"
#include "MarketDataEvents.h"
#include <variant>

template <class... EventTypes> struct overload : EventTypes... {
  using EventTypes::operator()...;
};

void MarketDataPublisher::Publish(const FeedMessage &ev) {

  MarketDataEvent marketDataEvent;
  marketDataEvent.seqNum = ev.seq;
  marketDataEvent.timeStamp = ev.timeStamp;

  std::visit(
      overload{[&](TradeEvent event) {
                 marketDataEvent.instrumentId = event.instrumentId;
                 marketDataEvent.body =
                     MdTrade{.price = event.restingPrice,
                             .qty = event.quantityTraded,
                             .agressorSide = event.aggressorSide,
                             .tradeId = event.tradeId};
                 m_transport.Send(marketDataEvent);
               },

               // We are only doing an L2 OrderBook for this project. So we
               // don't need these 2 events. They exist internally but we do not
               // need to send these out for external listeners. I'm keeping the
               // internal events for reference and in case i want to update in
               // future for learning purposes
               [&](OrderAddedEvent event) {}, [&](OrderRemovedEvent event) {},

               [&](LevelChangedEvent event) {
                 marketDataEvent.instrumentId = event.instrumentId;
                 marketDataEvent.body =
                     MdLevelUpdate{.side = event.side,
                                   .price = event.price,
                                   .totalQty = event.totalQty};
                 m_transport.Send(marketDataEvent);
               }},
      ev.payload);
}
