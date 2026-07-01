#include "FeedPump.h"
#include "FeedMessage.h"
#include "InternalEvents.h"

FeedPump::FeedPump(InternalEventQueue &internalQueue, Sequencer &sequencer,
                   std::vector<IMarketDataSink *> sinks)
    : m_internalEventQueue(internalQueue), m_sequencer(sequencer),
      m_sinks(sinks) {}

void FeedPump::Drain() {
  InternalEvent event;

  while (m_internalEventQueue.pop(event)) {
    FeedMessage msg = m_sequencer.Sequence(event);
    for (auto *sink : m_sinks) {
      sink->Publish(msg);
    }
  }
}
