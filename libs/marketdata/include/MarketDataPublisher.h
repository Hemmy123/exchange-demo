#pragma once

#include "IInternalEventSink.h"
#include "InternalEvents.h"
#include "MarketDataEvents.h"
#include "Types.h"

struct IMarketDataTransport {
  virtual void Send(const MarketDataEvent &ev) = 0;
  virtual ~IMarketDataTransport() = default;
};

class MarketDataPublisher : public IEventSink {
public:
  MarketDataPublisher(IMarketDataTransport &transport)
      : m_transport(transport) {};

  void Publish(const InternalEvent &ev) override;

private:
  SeqNum m_nextSeq{1}; // 0 reserved for invalid

  IMarketDataTransport &m_transport;
};
