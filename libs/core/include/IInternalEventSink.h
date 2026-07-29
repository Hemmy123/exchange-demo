#pragma once
#include "InternalEvents.h"

struct IInternalEventSink {
  virtual void Publish(const InternalEvent &ev) = 0;
  virtual ~IInternalEventSink() = default;
};
