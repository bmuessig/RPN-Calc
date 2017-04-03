#ifndef INTSTACK_H
#define INTSTACK_H

#include <Arduino.h>

typedef int stkint;

typedef struct {
  byte size;
  byte pointer;
  stkint* data;
} IntStack;

bool intStackInit(unsigned short size, IntStack* stack);
void intStackDestroy(IntStack* stack);
bool intStackGet(IntStack* stack, unsigned short index, int* value);
bool intStackPop(IntStack* stack, stkint* value);
bool intStackPush(stkint value, IntStack* stack);
bool intStackPeekAt(unsigned short offset, IntStack* stack, stkint* value);
bool intStackPeek(IntStack* stack, stkint* value);
bool intStackClear(IntStack* stack);

#endif
