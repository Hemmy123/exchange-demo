#pragma once

#include "FeedMessage.h"
#include "IMarketDataSink.h"
#include "IMarketDataTransport.h"

class MarketDataPublisher : public IMarketDataSink {
public:
  MarketDataPublisher(IMarketDataTransport &transport)
      : m_transport(transport) {};

  void Publish(const FeedMessage &ev) override;

private:
  IMarketDataTransport &m_transport;
};

///////////// scratch work /////////////////////
