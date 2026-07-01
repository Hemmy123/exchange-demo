#include "QueueProducer.h"
#include "InternalEvents.h"

void QueueProducer::Publish(const InternalEvent &event) {
  if (m_internalEventQueue.push(event)) {
    // TODO: handle full
  }
}
