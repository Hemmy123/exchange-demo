#pragma once

#include "IInternalEventSink.h"
#include "SpscRing.h"

class QueueProducer : public IInteralEventSink {
public:
  explicit QueueProducer(InternalEventQueue &queue)
      : m_internalEventQueue(queue) {}

  void Publish(const InternalEvent &event);

private:
  InternalEventQueue &m_internalEventQueue;
};
