#include "listTools.h"

const char* mergeItemPtr(byte index, const char** items1, byte itemCount1, const char** items2, byte itemCount2) {
  if(!items1 || (itemCount2 && !items2))
    return NULL;
  if(index >= itemCount1) {
    if(index - itemCount1 > itemCount2)
      return NULL;
    else
      return *(items2 + index - itemCount1);
  } else
    return *(items1 + index);
}

byte findClosestIndices(char* keyString, const char** items, byte itemCount, byte* indices, byte maxIndices) {
  return findClosestIndicesDualFiltered(keyString, items, itemCount, NULL, 0, NULL, 0, indices, maxIndices);
}

byte findClosestIndicesFiltered(char* keyString, const char** items, byte itemCount, byte* blacklist, byte blacklistCount, byte* indices, byte maxIndices) {
  return findClosestIndicesDualFiltered(keyString, items, itemCount, NULL, 0, blacklist, blacklistCount, indices, maxIndices);
}

byte findClosestIndicesDual(char* keyString, const char** items1, byte itemCount1, const char** items2, byte itemCount2, byte* indices, byte maxIndices, byte* blacklist, byte blacklistCount) {
  return findClosestIndicesDualFiltered(keyString, items1, itemCount1, items2, itemCount2, NULL, 0, indices, maxIndices);
}

byte findClosestIndicesDualFiltered(char* keyString, const char** items1, byte itemCount1, const char** items2, byte itemCount2, byte* blacklist, byte blacklistCount, byte* indices, byte maxIndices) {
  if(!keyString || !items1 || !itemCount1 || !indices || !maxIndices
    || (itemCount2 && !items2) || (blacklistCount && !blacklist))
    return 0; // invalid parameters will not produce any valid results

  byte indicesFound = 0, item = 0;
  while(indicesFound < maxIndices && item < itemCount1 + itemCount2) { // loop for as long as we have items or can find further items
    bool itemBlacklisted = false;
    // Check whether the item is blacklisted
    if(blacklistCount) {
      for(byte blItem = 0; blItem < blacklistCount; blItem++) {
        if(blacklist[blItem] == item) {
          itemBlacklisted = true;
          break;
        }
      }
    }
    Serial.print("New item: (0x");
    Serial.print((unsigned int)mergeItemPtr(item, items1, itemCount1, items2, itemCount2), HEX);
    Serial.print("): ");
    Serial.println(mergeItemPtr(item, items1, itemCount1, items2, itemCount2));

    if(strlen(mergeItemPtr(item, items1, itemCount1, items2, itemCount2)) < strlen(keyString) || itemBlacklisted) { // if the string is already longer than the item, skip it for speed
#ifdef DEBUG
      Serial.println("Skipping due to length");
#endif
      item++;
      continue;
    }
    // we now know that the current items length is smaller or equal to the key sequences length

    bool isMatching = true;
    for(byte chrPos = 0; chrPos < strlen(keyString); chrPos++) { // here we loop through the letters of the key string
      const char* itemPtr = mergeItemPtr(item, items1, itemCount1, items2, itemCount2);
      if(!itemPtr) {
        Serial.println("NULLPTR");
        return 0;
      }

      char itemChr = itemPtr[chrPos], keyChr = keyString[chrPos];
      byte mapSegment;

      // now, match the pressed key to a map segment
      if(!matchMapSegment(keyChr, &mapSegment))
        return 0; // key string is invalid

#ifdef DEBUG
      Serial.print("Key: ");
      Serial.println(keyChr);
      Serial.print("Item Chr: ");
      Serial.println(itemChr);
#endif

      // then check if the segment contains the items character
      if(!mapSegmentMatchesChar(itemChr, mapSegment)) {
        // it doesn't, so ...
        isMatching = false; // tell the succeeding code we don't have a match
        break; // and exit the string comparing loop
      }
      // it does, so ...
      // keep going
    }

    // did we have a match?
    if(isMatching)
      indices[indicesFound++] = item;

    // advance the item pointer
    item++;

    // and loop again (if possible)
  }

  return indicesFound;
}
