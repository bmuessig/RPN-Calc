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
        uiControl();
        bool doQuit = display.userInterfaceMessage("Quit?", "Really quit?", "", " No \n Yes ") == 2;
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
              case SOP_POW:
              case SOP_LOG:
              case SOP_EE:
              case SOP_SWP:
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
              case SOP_ABS:
              case SOP_NEG:
              case SOP_LN:
              case SOP_DUP:
              case SOP_POP:
              case SOP_SIN:
              case SOP_COS:
              case SOP_TAN:
              case SOP_ASIN:
              case SOP_ACOS:
              case SOP_ATAN:
              //case SOP_MACRO:
                if(!doubleStackPop(&stack, &val1)) {
                  error = STMATH_ERR_UNDERFLOW;
                  break;
                }
              break;
              case SOP_MACRO:
                error = STMATH_ERR_NOTIMPL;
              break;
            }

          if(!error) {
            switch(selection) {
                case SOP_ADD:
                  doubleStackPush(val2 + val1, &stack);
                break;
                case SOP_SUB:
                  doubleStackPush(val2 - val1, &stack);
                break;
                case SOP_MUL:
                  doubleStackPush(val2 * val1, &stack);
                break;
                case SOP_DIV:
                  if(val2 == 0) {
                    error = STMATH_ERR_DIVZERO;
                    break;
                  }
                  doubleStackPush(val2 / val1, &stack);
                break;
                case SOP_INC:
                  doubleStackPush(val1 + 1, &stack);
                break;
                case SOP_DEC:
                  doubleStackPush(val2 - 1, &stack);
                break;
                case SOP_POW:
                  doubleStackPush(pow(val2, val1), &stack);
                break;
                case SOP_EE:
                  doubleStackPush(val2 * pow(10, val1), &stack);
                break;
                case SOP_SQ:
                  doubleStackPush(sq(val1), &stack);
                break;
                case SOP_SQRT:
                  doubleStackPush(sqrt(val1), &stack);
                break;
                case SOP_LOG10:
                  doubleStackPush(log(val1) / log(10), &stack);
                break;
                case SOP_LOG:
                  doubleStackPush(log(val2) / log(val1), &stack);
                break;
                case SOP_ABS:
                  doubleStackPush(abs(val1), &stack);
                break;
                case SOP_NEG:
                  doubleStackPush(-val1, &stack);
                break;
                case SOP_LN:
                  doubleStackPush(log(val1), &stack);
                break;
                case SOP_PI:
                  doubleStackPush(PI, &stack);
                break;
                case SOP_E:
                  doubleStackPush(M_E, &stack);
                break;
                case SOP_TAU:
                  doubleStackPush(PI*2, &stack);
                break;
                case SOP_DUP:
                  doubleStackPush(val1, &stack);
                  doubleStackPush(val1, &stack);
                break;
                case SOP_POP:
                  // just put nothing back
                break;
                case SOP_SWP:
                  // put back in reverse order
                  doubleStackPush(val1, &stack);
                  doubleStackPush(val2, &stack);
                break;
                case SOP_SIN:
                  if(useRads)
                    doubleStackPush(sin(val1), &stack);
                  else
                    doubleStackPush(sin(degToRad(val1)), &stack);
                break;
                case SOP_COS:
                  if(useRads)
                    doubleStackPush(cos(val1), &stack);
                  else
                    doubleStackPush(cos(degToRad(val1)), &stack);
                break;
                case SOP_TAN:
                  if(useRads)
                    doubleStackPush(tan(val1), &stack);
                  else
                    doubleStackPush(tan(degToRad(val1)), &stack);
                break;
                case SOP_ASIN:
                  if(useRads)
                    doubleStackPush(asin(val1), &stack);
                  else
                    doubleStackPush(radToDeg(asin(val1)), &stack);
                break;
                case SOP_ACOS:
                  if(useRads)
                    doubleStackPush(acos(val1), &stack);
                  else
                    doubleStackPush(radToDeg(acos(val1)), &stack);
                break;
                case SOP_ATAN:
                  if(useRads)
                    doubleStackPush(atan(val1), &stack);
                  else
                    doubleStackPush(radToDeg(atan(val1)), &stack);
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
                case SOP_MACRO:
                  error = STMATH_ERR_NOTIMPL;
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

double radToDeg(double degVal) {
  return degVal * (180 / PI);
}

double degToRad(double radVal) {
  return radVal * (PI / 180);
}
