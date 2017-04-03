#include "intStack.h"

// Minimal double stack implementation
bool intStackInit(unsigned short size, IntStack* stack) {
  if(!stack || !size)
    return false;
  stack->size = size;
  stack->pointer = 0;
  return ((stack->data = (stkint*)malloc(size * sizeof(stkint))) != NULL);
}

void intStackDestroy(IntStack* stack) {
  if(!stack)
    return;
  if(!(stack->data))
    return;
  stack->size = 0;
  stack->pointer = 0;
  free(stack->data);
}

bool intStackGet(IntStack* stack, unsigned short index, stkint* value) {
  if(!stack || !value)
    return false;
  if(index >= stack->pointer)
    return false;
  *value = stack->data[index];
  return true;
}

// This is the only function where value may be NULL (to drop a value)
bool intStackPop(IntStack* stack, stkint* value) {
  if(!stack)
    return false;
  if(!(stack->data) || !stack->size || !stack->pointer)
    return false;
  (stack->pointer)--;
  if(!value)
    *value = stack->data[stack->pointer];
  return true;
}

bool intStackPush(stkint value, IntStack* stack) {
  if(!stack)
    return false;
  if(!(stack->data) || !stack->size || stack->pointer >= stack->size)
    return false;
  stack->data[(stack->pointer)++] = value;
  return true;
}

bool intStackPeekAt(unsigned short offset, IntStack* stack, stkint* value) {
  if(!stack || !value)
    return false;
  if(!(stack->data) || !(stack->size) || offset + 1 > stack->pointer)
    return false;
  *value = stack->data[stack->pointer - offset - 1];
  return true;
}

bool intStackPeek(IntStack* stack, stkint* value) {
  return intStackPeekAt(0, stack, value);
}

bool intStackClear(IntStack* stack) {
  if(!stack)
    return false;
  if(!(stack->data) || !(stack->size))
    return false;
  stack->pointer = 0;
  return true;
}
