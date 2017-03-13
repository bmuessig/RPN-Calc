#include "hardware.h"

const byte lcdDC = 14, lcdCS = 8, lcdRS = 15, lcdBL = 9;
byte lcdContrast = 140, lcdBrightness = 255;
U8G2_PCD8544_84X48_F_4W_HW_SPI display(U8G2_R0, lcdCS, lcdDC, lcdRS);

const byte keypadRows[4] = { 5,  4,  3,  2};
const byte keypadCols[3] = {23, 22, 21};
Keypad keypad = Keypad( makeKeymap((char*)keymap), (byte*)keypadRows, (byte*)keypadCols, 4, 3 );

const byte led = A14;
byte ledBrightness = 200;

File sdRoot;
const byte sdCS = 16;
