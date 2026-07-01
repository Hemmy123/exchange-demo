#pragma once

// Owns the sequencer and fans out the various sinks
#include "IMarketDataSink.h"
#include "Sequencer.h"
#include "SpscRing.h"
#include <vector>

class FeedPump {
public:
  FeedPump(InternalEventQueue &internalQueue, Sequencer &sequencer,
           std::vector<IMarketDataSink *> sinks);

  void Drain();

private:
  InternalEventQueue &m_internalEventQueue;
  Sequencer &m_sequencer;
  std::vector<IMarketDataSink *> m_sinks;
};
