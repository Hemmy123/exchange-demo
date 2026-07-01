#pragma once

#include "FeedMessage.h"
#include "InternalEvents.h"

class Sequencer {
public:
  FeedMessage Sequence(const InternalEvent &event);

private:
  SeqNum m_nextSeq{1}; // 0 reserved for invalid
};
