#pragma once

#include "IInternalEventSink.h"
#include "SpscRing.h"

class QueueProducer : public IInternalEventSink {
public:
  explicit QueueProducer(InternalEventQueue &queue)
      : m_internalEventQueue(queue) {}

  void Publish(const InternalEvent &event) override;

private:
  InternalEventQueue &m_internalEventQueue;
};
