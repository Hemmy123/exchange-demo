#pragma once
#include "InternalEvents.h"

struct IEventSink {
  virtual void Publish(const InternalEvent &ev) = 0;
  virtual ~IEventSink() = default;
};
