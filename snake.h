#ifndef SNAKE_H
#define SNAKE_H

#include <Arduino.h>
#include "keyUtils.h"
#include "textView.h"
#include "schedule.h"

#include "snakeGlob.h"

// Candy states
enum {
  SNAKE_CANDY_PENDING,
  SNAKE_CANDY_PRESENT
};

// Directions
enum {
  SNAKE_DIR_NONE  = 0b0000,

  SNAKE_DIR_HORIZ = 0b0001,
  SNAKE_DIR_EAST  = 0b0001,
  SNAKE_DIR_WEST  = 0b0011,

  SNAKE_DIR_VERT  = 0b0100,
  SNAKE_DIR_NORTH = 0b0100,
  SNAKE_DIR_SOUTH = 0b1100,

  SNAKE_DIR_NE    = SNAKE_DIR_NORTH | SNAKE_DIR_EAST,
  SNAKE_DIR_NW    = SNAKE_DIR_NORTH | SNAKE_DIR_WEST,

  SNAKE_DIR_SE    = SNAKE_DIR_SOUTH | SNAKE_DIR_EAST,
  SNAKE_DIR_SW    = SNAKE_DIR_SOUTH | SNAKE_DIR_WEST
};

void snakeGame(void);
void snakeHighScores(void);

#endif
