#ifndef STACKMATH_H
#define STACKMATH_H

#include <Arduino.h>
#include <math.h>
#include "RPN-Calc.h"
#include "textView.h"
#include "keyUtils.h"
#include "userInterface.h"
#include "doubleStack.h"

#include "stackMathGlob.h"

enum {
  STMATH_ERR_NONE,
  STMATH_ERR_UNDERFLOW,
  STMATH_ERR_OVERFLOW,
  STMATH_ERR_DIVZERO,
  STMATH_ERR_SQRTNEG,
  STMATH_ERR_NOTIMPL,
  STMATH_ERR_UNKNOWN
} stackMathErrors;

const char* stackMathOpNames[] = {
  "ADD",
  "SUB",
  "MUL",
  "DIV",
  "INC",
  "DEC",
  "POW",
  "EE",
  "SQ",
  "SQRT",
  "LOG10",
  "LOG",
  "ABS",
  "NEG",
  "LN",
  "PI",
  "E",
  "TAU",
  "DUP",
  "POP",
  "SWP",
  "SIN",
  "COS",
  "TAN",
  "DEG",
  "RAD",
  "ASIN",
  "ACOS",
  "ATAN",
  "FLAGS",
  "MACRO"
};

enum {
  SOP_ADD,
  SOP_SUB,
  SOP_MUL,
  SOP_DIV,
  SOP_INC,
  SOP_DEC,
  SOP_POW,
  SOP_EE,
  SOP_SQ,
  SOP_SQRT,
  SOP_LOG10,
  SOP_LOG,
  SOP_ABS,
  SOP_NEG,
  SOP_LN,
  SOP_PI,
  SOP_E,
  SOP_TAU,
  SOP_DUP,
  SOP_POP,
  SOP_SWP,
  SOP_SIN,
  SOP_COS,
  SOP_TAN,
  SOP_DEG,
  SOP_RAD,
  SOP_ASIN,
  SOP_ACOS,
  SOP_ATAN,
  SOP_FLAGS,
  SOP_MACRO,
  SOP_ELEMENT_COUNT
} stackMathOps;

double radToDeg(double degVal);
double degToRad(double radVal);

#endif
