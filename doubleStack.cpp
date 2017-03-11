#include "doubleStack.h"

// Minimal double stack implementation
bool doubleStackInit(byte size, DoubleStack* stack) {
  if(!stack || !size)
    return false;
  stack->size = size;
  stack->pointer = 0;
  return ((stack->data = (double*)malloc(size * sizeof(double))) != NULL);
}

void doubleStackDestroy(DoubleStack* stack) {
  if(stack == NULL)
    return;
  if(stack->data == NULL)
    return;
  stack->size = 0;
  stack->pointer = 0;
  free(stack->data);
}

// This is the only function where value may be NULL (to drop a value)
bool doubleStackPop(DoubleStack* stack, double* value) {
  if(stack == NULL)
    return false;
  if(stack->data == NULL || !stack->size || !stack->pointer)
    return false;
  (stack->pointer)--;
  if(value != NULL)
    *value = stack->data[stack->pointer];
  return true;
}

bool doubleStackPush(double value, DoubleStack* stack) {
  if(stack == NULL)
    return false;
  if(stack->data == NULL || !stack->size || stack->pointer >= stack->size)
    return false;
  stack->data[(stack->pointer)++] = value;
  return true;
}

bool doubleStackPeekAt(byte offset, DoubleStack* stack, double* value) {
  if(stack == NULL || value == NULL)
    return false;
  if(stack->data == NULL || !(stack->size) || offset + 1 > stack->pointer)
    return false;
  *value = stack->data[stack->pointer - offset - 1];
  return true;
}

bool doubleStackPeek(DoubleStack* stack, double* value) {
  return doubleStackPeekAt(0, stack, value);
}

bool doubleStackClear(DoubleStack* stack) {
  if(stack == NULL)
    return false;
  if(stack->data == NULL || !(stack->size))
    return false;
  stack->pointer = 0;
  return true;
}
