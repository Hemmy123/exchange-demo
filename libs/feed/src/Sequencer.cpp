#include "Sequencer.h"
#include "FeedMessage.h"
#include "InternalEvents.h"

FeedMessage Sequencer::Sequence(const InternalEvent &event) {
  FeedMessage newMessage;
  newMessage.seq = m_nextSeq++;
  newMessage.timeStamp = std::chrono::system_clock::now();
  newMessage.payload = event;

  return newMessage;
}
