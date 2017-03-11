#include "stackMath.h"

void stackMath(void) {
  textViewFullscreen = false;
  textViewClear();
  keyControl();
  textViewSetTitle("Stack Math");
  textViewStatusUpdate();
  textViewLinefeed();
  textViewRender();

  const byte stackSize = 16;
  char key;
  bool doRedraw = true, useRads = false;
  DoubleStack stack;
  if(!doubleStackInit(stackSize, &stack)) {
    // We don't have enough RAM left to do anything; shouldn't ever happen
    return;
  }

  while(true) {
    if(key = keypad.getKey()) {
      if(key >= '0' && key <= '9') { // enter numbers
        double value = (double)(key - '0');
        textViewClear();
        textViewStatusUpdate();
        textViewSet(7, 0, TV_COLOR_F1H0);
        textViewPutCStr("*:BKSP/-/EX, #:DP/ENT");
        textViewSet(2, 0, TV_COLOR_F1H0);
        textViewPutCStr("Push value:\n");
        if(doubleEntry(&value, 18)) {
          if(!doubleStackPush(value, &stack)) {
            uiControl();
            display.userInterfaceMessage("Error:", "Stack overflow!", "", " OK ");
          }
        }
        doRedraw = true;
        keyControl();
      } else if(key == '*') { // exit
        bool doQuit = false;

        uiControl();
        switch (display.userInterfaceMessage("Quit, reset", "or go back?", "", "Back\nQuit\nReset")) {
          case 2:
            doQuit = true;
          break;
          case 3:
            doubleStackClear(&stack);
          break;
        }
        keyControl();

        if(doQuit)
          break;
        else
          doRedraw = true;
      } else if(key == '#') { // operation menu
        byte selection;
        if(smartMenu("Select operation:", stackMathOpNames, SOP_ELEMENT_COUNT, SOP_ELEMENT_COUNT, &selection)) {
          double val1 = NAN, val2 = NAN;
          byte error = STMATH_ERR_NONE;

          // Fill the values
            switch(selection) {
              // Two values popped
              case SOP_ADD:
              case SOP_SUB:
              case SOP_MUL:
              case SOP_DIV:
              case SOP_MOD:
              case SOP_POW:
              case SOP_EE:
              case SOP_LOG:
              case SOP_SWP:
              case SOP_ROUND:
              case SOP_MIN:
              case SOP_MAX:
                if(!doubleStackPop(&stack, &val1)) {
                  error = STMATH_ERR_UNDERFLOW;
                  break;
                }
                if(!doubleStackPop(&stack, &val2)) {
                  doubleStackPush(val1, &stack);
                  error = STMATH_ERR_UNDERFLOW;
                  break;
                }
              break;
              // One value popped
              case SOP_INC:
              case SOP_DEC:
              case SOP_SQ:
              case SOP_SQRT:
              case SOP_LOG10:
              case SOP_LN:
              case SOP_FAC:
              case SOP_ABS:
              case SOP_NEG:
              case SOP_SGN:
              case SOP_INV:
              case SOP_SIN:
              case SOP_COS:
              case SOP_TAN:
              case SOP_ASIN:
              case SOP_ACOS:
              case SOP_ATAN:
              case SOP_SINH:
              case SOP_COSH:
              case SOP_TANH:
              case SOP_ASINH:
              case SOP_ACOSH:
              case SOP_ATANH:
              case SOP_DUP:
              case SOP_CPY:
              case SOP_POP:
              case SOP_IROUND:
              case SOP_FLOOR:
              case SOP_CEIL:
              case SOP_FPART:
                if(!doubleStackPop(&stack, &val1)) {
                  error = STMATH_ERR_UNDERFLOW;
                  break;
                }
              break;
            }

          if(!error) {
            switch(selection) {
                case SOP_ADD:
                  stackMathPush(val2 + val1, &error, &stack);
                break;
                case SOP_SUB:
                  stackMathPush(val2 - val1, &error, &stack);
                break;
                case SOP_MUL:
                  stackMathPush(val2 * val1, &error, &stack);
                break;
                case SOP_DIV:
                  if(val2 == 0) {
                    error = STMATH_ERR_DIVZERO;
                    break;
                  }
                  stackMathPush(val2 / val1, &error, &stack);
                break;
                case SOP_MOD:
                  if(val2 == 0) {
                    error = STMATH_ERR_DIVZERO;
                    break;
                  }
                  stackMathPush(fmod(val2, val1), &error, &stack);
                break;
                case SOP_FRX:
                  int a, b;
                  textViewClear();
                  textViewStatusUpdate();
                  textViewSet(2, 0, TV_COLOR_F1H0);
                  textViewPutCStr("Fraction a / b\na: ");
                  if(!intEntry(&a, true))
                    break;
                  textViewPutCStr("b: ");
                  if(!intEntry(&b, false))
                    break;
                  keyControl();
                  if(b == 0) {
                    error = STMATH_ERR_DIVZERO;
                    break;
                  }
                  stackMathPush((double)((double)a / (double)b), &error, &stack);
                break;
                case SOP_INC:
                  stackMathPush(val1 + 1, &error, &stack);
                break;
                case SOP_DEC:
                  stackMathPush(val1 - 1, &error, &stack);
                break;
                case SOP_POW:
                  stackMathPush(pow(val2, val1), &error, &stack);
                break;
                case SOP_EE:
                  stackMathPush(val2 * pow(10, val1), &error, &stack);
                break;
                case SOP_SQ:
                  stackMathPush(sq(val1), &error, &stack);
                break;
                case SOP_SQRT:
                  if(val1 < 0) {
                    error = STMATH_ERR_SQRTNEG;
                    break;
                  }
                  stackMathPush(sqrt(val1), &error, &stack);
                break;
                case SOP_LOG10:
                  stackMathPush(log(val1) / log(10), &error, &stack);
                break;
                case SOP_LN:
                  stackMathPush(log(val1), &error, &stack);
                break;
                case SOP_LOG:
                  stackMathPush(log(val2) / log(val1), &error, &stack);
                break;
                case SOP_FAC:
                  if(val1 < 0) {
                    error = STMATH_ERR_RANGE;
                    break;
                  }
                  if(round(val1) != val1) {
                    error = STMATH_ERR_NOTINT;
                    break;
                  }
                  stackMathPush(fact(val1), &error, &stack);
                break;
                case SOP_ABS:
                  stackMathPush(abs(val1), &error, &stack);
                break;
                case SOP_NEG:
                  stackMathPush(-val1, &error, &stack);
                break;
                case SOP_SGN:
                  stackMathPush(val1 / abs(val1), &error, &stack);
                break;
                case SOP_INV:
                  stackMathPush(1 / val1, &error, &stack);
                break;
                case SOP_SIN:
                  if(useRads)
                    stackMathPush(sin(val1), &error, &stack);
                  else
                    stackMathPush(sin(degToRad(val1)), &error, &stack);
                break;
                case SOP_COS:
                  if(useRads)
                    stackMathPush(cos(val1), &error, &stack);
                  else
                    stackMathPush(cos(degToRad(val1)), &error, &stack);
                break;
                case SOP_TAN:
                  if(useRads)
                    stackMathPush(tan(val1), &error, &stack);
                  else
                    stackMathPush(tan(degToRad(val1)), &error, &stack);
                break;
                case SOP_ASIN:
                  if(useRads)
                    stackMathPush(asin(val1), &error, &stack);
                  else
                    stackMathPush(radToDeg(asin(val1)), &error, &stack);
                break;
                case SOP_ACOS:
                  if(useRads)
                    stackMathPush(acos(val1), &error, &stack);
                  else
                    stackMathPush(radToDeg(acos(val1)), &error, &stack);
                break;
                case SOP_ATAN:
                  if(useRads)
                    stackMathPush(atan(val1), &error, &stack);
                  else
                    stackMathPush(radToDeg(atan(val1)), &error, &stack);
                break;
                case SOP_SINH:
                  if(useRads)
                    stackMathPush(sinh(val1), &error, &stack);
                  else
                    stackMathPush(sinh(degToRad(val1)), &error, &stack);
                break;
                case SOP_COSH:
                  if(useRads)
                    stackMathPush(cosh(val1), &error, &stack);
                  else
                    stackMathPush(cosh(degToRad(val1)), &error, &stack);
                break;
                case SOP_TANH:
                  if(useRads)
                    stackMathPush(tanh(val1), &error, &stack);
                  else
                    stackMathPush(tanh(degToRad(val1)), &error, &stack);
                  break;
                case SOP_ASINH:
                  if(useRads)
                    stackMathPush(asinh(val1), &error, &stack);
                  else
                    stackMathPush(radToDeg(asinh(val1)), &error, &stack);
                case SOP_ACOSH:
                  if(useRads)
                    stackMathPush(acosh(val1), &error, &stack);
                  else
                    stackMathPush(radToDeg(acosh(val1)), &error, &stack);
                case SOP_ATANH:
                  if(useRads)
                    stackMathPush(atanh(val1), &error, &stack);
                  else
                    stackMathPush(radToDeg(atanh(val1)), &error, &stack);
                case SOP_PI:
                  stackMathPush(PI, &error, &stack);
                break;
                case SOP_E:
                  stackMathPush(M_E, &error, &stack);
                break;
                case SOP_TAU:
                  stackMathPush(PI * 2, &error, &stack);
                break;
                case SOP_RAND:
                  stackMathPush(randDouble(), &error, &stack);
                break;
                case SOP_DUP:
                  stackMathPush(val1, &error, &stack);
                  stackMathPush(val1, &error, &stack);
                break;
                case SOP_CPY:
                  if(doubleStackPeekAt(val1, &stack, &val2)) {
                    stackMathPush(val2, &error, &stack);
                  } else
                    error = STMATH_ERR_RANGE;
                break;
                case SOP_POP:
                  // just put nothing back
                break;
                case SOP_SWP:
                  // put back in reverse order
                  stackMathPush(val1, &error, &stack);
                  stackMathPush(val2, &error, &stack);
                break;
                case SOP_CLR:
                  error = doubleStackClear(&stack) ? STMATH_ERR_NONE : STMATH_ERR_UNKNOWN;
                break;
                case SOP_ROUND:
                  if(round(val1) != val1) {
                    error = STMATH_ERR_NOTINT;
                    break;
                  }
                  stackMathPush(roundTo(val2, (byte)round(val1)), &error, &stack);
                break;
                case SOP_IROUND:
                  stackMathPush(round(val1), &error, &stack);
                break;
                case SOP_FLOOR:
                  stackMathPush(floor(val1), &error, &stack);
                break;
                case SOP_CEIL:
                  stackMathPush(ceil(val1), &error, &stack);
                break;
                case SOP_FPART:
                  stackMathPush(val1 - floor(val1), &error, &stack);
                break;
                case SOP_MIN:
                  stackMathPush(val1 < val2 ? val1 : val2, &error, &stack);
                break;
                case SOP_MAX:
                  stackMathPush(val1 > val2 ? val1 : val2, &error, &stack);
                break;
                case SOP_DEG:
                  useRads = false;
                break;
                case SOP_RAD:
                  useRads = true;
                break;
                case SOP_FLAGS:
                  uiControl();
                  uiMessagePrintf("Angle Mode: %s", " OK ", useRads ? "RAD" : "DEG");
                  keyControl();
                break;
                default:
                  error = STMATH_ERR_UNKNOWN;
                break;
              }
          }

          uiControl();
          switch(error) {
            case STMATH_ERR_NONE:
            break;
            case STMATH_ERR_UNDERFLOW:
              display.userInterfaceMessage("Error:", "Stack underflow!", "", " OK ");
            break;
            case STMATH_ERR_OVERFLOW:
              display.userInterfaceMessage("Error:", "Stack overflow!", "", " OK ");
            break;
            case STMATH_ERR_RANGE:
              display.userInterfaceMessage("Error:", "Input out of range!", "", " OK ");
            break;
            case STMATH_ERR_NOTINT:
              display.userInterfaceMessage("Error:", "Input not an integer!", "", " OK ");
            break;
            case STMATH_ERR_DIVZERO:
              display.userInterfaceMessage("Error:", "Division by 0!", "", " OK ");
            break;
            case STMATH_ERR_SQRTNEG:
              display.userInterfaceMessage("Error:", "Sqrt of neg. num!", "", " OK ");
            break;
            case STMATH_ERR_NOTIMPL:
              display.userInterfaceMessage("Error:", "Not implemented!", "", " OK ");
            break;
            case STMATH_ERR_UNKNOWN:
            default:
              display.userInterfaceMessage("Error:", "Unknown error!", "", " OK ");
            break;
          }
        }
        doRedraw = true;
        keyControl();
      }
    }

    if(doRedraw) {
      doRedraw = false;
      textViewClear();

      // Draw the stack
      double stackVal;

      if(stack.pointer) {
        textViewSet(1, 0, TV_COLOR_F1H0);
#ifdef DEBUG
        Serial.print("Stackptr: ");
        Serial.println(stack.pointer, DEC);
#endif
        for(byte ptr = 0; ptr < constrain(stack.pointer, 0, 6); ptr++) {
#ifdef DEBUG
          Serial.print("Pointer: ");
          Serial.println(ptr, DEC);
          Serial.print("Success: ");
          Serial.println(doubleStackPeekAt(ptr, &stack, &stackVal), DEC);
          Serial.print("Value: ");
          Serial.println(stackVal, DEC);
#endif
          if(doubleStackPeekAt(ptr, &stack, &stackVal))
            textViewNPrintf("%f\n", 21, stackVal);
          else
            textViewPutCStr("nan\n");
        }
      }
      textViewSet(7, 0, TV_COLOR_F1H0);
      textViewPutCStr("*:EXIT, 0-9:NUM, #:OP");

      textViewStatusUpdate();
      textViewRender();
    }

    doEvents();
  }

  doubleStackDestroy(&stack);
  textViewClearAll();
  uiControl();
}

void stackMathPush(double value, byte* error, DoubleStack* stack) {
  if(!*error)
    *error = doubleStackPush(value, stack) ? STMATH_ERR_NONE : STMATH_ERR_OVERFLOW;
}

double radToDeg(double degVal) {
  return degVal * (180 / PI);
}

double degToRad(double radVal) {
  return radVal * (PI / 180);
}

double randDouble(void) {
  return ((double)rand()/(double)RAND_MAX);
}

double fact(double x) {
  return tgamma(x+1.);
}

double roundTo(double x, byte digits) {
    double fac = pow(10, digits);
    return round(x*fac)/fac;
}
