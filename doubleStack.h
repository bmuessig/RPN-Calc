#ifndef DOUBLE_STACK_H
#define DOUBLE_STACK_H

#include <Arduino.h>

typedef struct {
  byte size;
  byte pointer;
  double* data;
} DoubleStack;

bool doubleStackInit(byte size, DoubleStack* stack);
void doubleStackDestroy(DoubleStack* stack);
bool doubleStackPop(DoubleStack* stack, double* value);
bool doubleStackPush(double value, DoubleStack* stack);
bool doubleStackPeekAt(byte offset, DoubleStack* stack, double* value);
bool doubleStackPeek(DoubleStack* stack, double* value);

#endif
