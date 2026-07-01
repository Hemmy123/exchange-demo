#pragma once
#include "InternalEvents.h"

struct IInteralEventSink {
  virtual void Publish(const InternalEvent &ev) = 0;
  virtual ~IInteralEventSink() = default;
};
