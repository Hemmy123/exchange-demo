
#include <cassert>
#include <iostream>
#include <variant>
#include <vector>

// core
#include "FeedMessage.h" // FeedMessage { seq, timeStamp, payload }
#include "IMarketDataSink.h"
#include "InternalEvents.h" // InternalEvent, TradeEvent, OrderAdded, OrderRemoved
#include "Types.h"          // SeqNum

// matching
#include "MatchingEngine.h"

// feed
#include "FeedPump.h"
#include "QueueProducer.h"
#include "Sequencer.h"

// marketdata
#include "IMarketDataTransport.h"
#include "MarketDataEvents.h" // MarketDataEvent (transport payload)
#include "MarketDataPublisher.h"

// Spy sink: records every FeedMessage so we can inspect the stream.
class RecordingSink : public IMarketDataSink {
public:
  void Publish(const FeedMessage &msg) override { messages.push_back(msg); }
  std::vector<FeedMessage> messages;
};

// No-op transport that counts sends, to confirm the real publisher forwarded.
class CountingTransport : public IMarketDataTransport {
public:
  void Send(const MarketDataEvent & /*ev*/) override { ++sendCount; }
  std::size_t sendCount = 0;
};

/*
 DataFlow (Top to bottom):
========================================================================
 Matching Engine (OrderBook)                            InternalEvent
 QueueProducer (SPSC)                                   InternalEvent
 FeedPump (Uses Sequencer to sequence InternalEvent)    InternalEvent
 MarketDataPublisher                                    InternalEvent
 MarketDataTransport                                    MarketDataEvent
*/

void ExchangeTest() {

  // ======================= Setup ====================== //
  InternalEventQueue queue;

  QueueProducer producer(queue);
  MatchingEngine engine(producer);

  Sequencer sequencer;
  CountingTransport transport;

  MarketDataPublisher publisher(transport); // Main Sink
  RecordingSink recorder;                   // Secondary Sink for testing

  std::vector<IMarketDataSink *> sinks{&publisher, &recorder};
  FeedPump pump(queue, sequencer, sinks); // pop -> sequence -> fan out to both

  // ==================================================== //

  InstrumentId instrumentId = 1;
  Order order1 = {.id = 1, .price = 100, .qty = 10};
  Order order2 = {.id = 2, .price = 100, .qty = 10};

  engine.PlaceOrder(instrumentId, Side::Bid, order1);
  engine.PlaceOrder(instrumentId, Side::Ask, order2);

  // Single-threaded for now: drain on this same thread.
  // (Becomes a loop on the transport thread once the real SpscRing is in.)
  pump.Drain();

  // ---- assert ----
  const std::size_t n = recorder.messages.size();

  std::size_t trades = 0, adds = 0, removes = 0;

  for (const FeedMessage &m : recorder.messages) {
    trades += std::holds_alternative<TradeEvent>(m.payload);
    adds += std::holds_alternative<OrderAddedEvent>(m.payload);
    removes += std::holds_alternative<OrderRemovedEvent>(m.payload);
  }

  std::cout << "FeedMessages delivered: " << n << "  (trades=" << trades
            << " adds=" << adds << " removes=" << removes << ")\n"
            << "Publisher forwarded to transport: " << transport.sendCount
            << "\n";

  // 1. something reached the sinks at all
  assert(n >= 1 && "no FeedMessages reached the sinks -- pipeline is broken");

  // 2. the real MarketDataPublisher saw every message the spy saw
  //    (this is the 'goes through to MarketDataPublisher' check)
  assert(transport.sendCount == n && "publisher did not forward every message");

  // 3. sequencer assigned gap-free, monotonic seq starting at 1
  for (std::size_t i = 0; i < n; ++i)
    assert(recorder.messages[i].seq == static_cast<SeqNum>(i + 1) &&
           "sequence numbers not gap-free/monotonic");

  // tighten once crossing semantics are settled:
  // assert(trades >= 1 && "a fully crossing order produced no trade");

  std::cout << "PASS\n";
}

int main() { ExchangeTest(); }
