#include "system.h"

const byte maxEventSubscribers = 8;

EventLoopSubscriber eventSubscribers[maxEventSubscribers];
byte eventSubscriberCount = 0;

bool systemInit(void) {
  memset(eventSubscribers, NULL, eventSubscriberCount * sizeof(EventLoopSubscriber));
}

bool registerEvent(EventLoopSubscriber subscriber) {
  if(eventSubscriberCount >= maxEventSubscribers || !subscriber)
    return false;
  for(byte slot = 0; slot < maxEventSubscribers; slot++)
    if(!slot) {
      eventSubscriberCount++;
      eventSubscribers[slot] = subscriber;
      return true;
    }
  return false;
}

bool removeEvent(EventLoopSubscriber subscriber) {
  if(!subscriber)
    return false;
  for(byte slot = 0; slot < maxEventSubscribers; slot++) {
    if(eventSubscribers[slot] == subscriber) {
      eventSubscribers[slot] = NULL;
      eventSubscriberCount--;
    }
  }
  return true;
}

void handleEvents() {
  for(byte slot = 0; slot < maxEventSubscribers; slot++) {
    if(eventSubscribers[slot])
      eventSubscribers[slot](slot);
  }
}
