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

// Errors
enum {
  STMATH_ERR_NONE,
  STMATH_ERR_UNDERFLOW,
  STMATH_ERR_OVERFLOW,
  STMATH_ERR_RANGE,
  STMATH_ERR_NOTINT,
  STMATH_ERR_DIVZERO,
  STMATH_ERR_SQRTNEG,
  STMATH_ERR_ABORTED,
  STMATH_ERR_NOTIMPL,
  STMATH_ERR_UNKNOWN
};

const char* stackMathOps[] = {
  "ADD",    //
  "SUB",    //
  "MUL",    //
  "DIV",    //
  "MOD",    //
  "FRX",    //
  "INC",    //
  "DEC",    //
  "POW",    //
  "EE",     //
  "SQ",     //
  "SQRT",   //
  "LOG10",  //
  "LN",     //
  "LOG",    //
  "FAC",    //
  "ABS",    //
  "NEG",    //
  "SGN",    //
  "INV",    //
  "SIN",    //
  "COS",    //
  "TAN",    //
  "ASIN",   //
  "ACOS",   //
  "ATAN",   //
  "SINH",   //
  "COSH",   //
  "TANH",   //
  "ASINH",  //
  "ACOSH",  //
  "ATANH",  //
  "PI",     //
  "E",      //
  "TAU",    //
  "RAND",   //
  "DUP",    //
  "CPY",    //
  "POP",    //
  "SWP",    //
  "CLR",    //
  "ROUND",  //
  "IROUND", //
  "FLOOR",  //
  "CEIL",   //
  "FPART",  //
  "MIN",    //
  "MAX",    //
  "CONV",   //
  "DEG",    //
  "RAD",    //
  "FLAGS"   //
};

// Operators
enum {
  SOP_ADD,
  SOP_SUB,
  SOP_MUL,
  SOP_DIV,
  SOP_MOD,
  SOP_FRX,
  SOP_INC,
  SOP_DEC,
  SOP_POW,
  SOP_EE,
  SOP_SQ,
  SOP_SQRT,
  SOP_LOG10,
  SOP_LN,
  SOP_LOG,
  SOP_FAC,
  SOP_ABS,
  SOP_NEG,
  SOP_SGN,
  SOP_INV,
  SOP_SIN,
  SOP_COS,
  SOP_TAN,
  SOP_ASIN,
  SOP_ACOS,
  SOP_ATAN,
  SOP_SINH,
  SOP_COSH,
  SOP_TANH,
  SOP_ASINH,
  SOP_ACOSH,
  SOP_ATANH,
  SOP_PI,
  SOP_E,
  SOP_TAU,
  SOP_RAND,
  SOP_DUP,
  SOP_CPY,
  SOP_POP,
  SOP_SWP,
  SOP_CLR,
  SOP_ROUND,
  SOP_IROUND,
  SOP_FLOOR,
  SOP_CEIL,
  SOP_FPART,
  SOP_MIN,
  SOP_MAX,
  SOP_CONV,
  SOP_DEG,
  SOP_RAD,
  SOP_FLAGS,
  SOP_ELEMENT_COUNT
};

const char* stackMathConvNames[] = {
  // Pressure
  "BAR",
  "KG/",
  "a",
  "b",
  "c",
  "d"
};

void stackMathPush(double value, byte* error, DoubleStack* stack);

void stackMathConvert(double value, byte* error, DoubleStack* stack);
void stackMathFraction(byte* error, DoubleStack* stack);

double radToDeg(double degVal);
double degToRad(double radVal);
double randDouble(void);
double fact(double x);
double roundTo(double x, byte digits);

#endif
