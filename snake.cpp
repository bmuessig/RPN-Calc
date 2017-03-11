#include "snake.h"

void snake(void) {
  bool stillAlive = true;

  while(stillAlive) {
    uiControl();
    switch(display.userInterfaceSelectionList("Snake", 1, "Play\nHigh scores\nQuit")) {
      case 1:
        keyControl();
        snakeGame();
      break;
      case 2:
        keyControl();
        snakeHighScores();
      break;
      case 3:
        stillAlive = false;
      break;
    }
  }

  textViewClearAll();
  textViewRender();
  uiControl();
}

void snakeGame(void) {
  textViewFullscreen = true;
  textViewClear();
  keyControl();

  const byte maxSnakeLength = 20, maxCandy = 4;
  byte snake[maxSnakeLength][2] = {0}, candy[maxCandy][4] = {0}, snakeLength = 1, direction = SNAKE_DIR_NONE;
  Schedule logicSchedule;
  unsigned int score = 0;
  bool doRedraw = true, isRunning = true;
  char key;

  // Initialize the scheduler to run every 500 ms
  scheduleInit(500, NULL, &logicSchedule);

  while(isRunning) {
    // Check for key presses every cycle
    switch(key = keypad.getKey()) {
      case '1':
        direction = SNAKE_DIR_NW;
      break;
      case '2':
        direction = SNAKE_DIR_NORTH;
      break;
      case '3':
        direction = SNAKE_DIR_NE;
      break;
      case '4':
        direction = SNAKE_DIR_WEST;
      break;
      case '5':
        direction = SNAKE_DIR_NONE; // this is essentially cheating; will be removed later
      break;
      case '6':
        direction = SNAKE_DIR_EAST;
      break;
      case '7':
        direction = SNAKE_DIR_SW;
      break;
      case '8':
        direction = SNAKE_DIR_SOUTH;
      break;
      case '9':
        direction = SNAKE_DIR_SE;
      break;
      case '0':
        // not yet defined; maybe some special action?
      break;
      case '*':
        // will exit the game
        isRunning = false;
      break;
      case '#':
        // will pause the game
      break;
    }

    // Run the game logic every n-milliseconds
    if(scheduleRun(&logicSchedule)) {
      bool snakeBitten = false, candyEaten = false;
      byte newRow = snake[0][0], newCol = snake[0][1];

      // Update the position of the first element
      if(direction & SNAKE_DIR_EAST)
        newCol++;
      else if(direction & SNAKE_DIR_WEST)
        newCol--;
      if(direction & SNAKE_DIR_SOUTH)
        newRow++;
      else if(direction & SNAKE_DIR_NORTH)
        newCol--;

      // Check if we are either biting ourselves or eating candy
      for(byte segment = 1; segment < snakeLength; segment++) {
        if(snake[segment][0] == newRow && snake[segment][1] == newCol) {
          // collision
          snakeBitten = true;
          break;
        }
      }
      for(byte berry = 0; berry < maxCandy && !snakeBitten; berry++) {
        if(candy[berry][0] == SNAKE_CANDY_PRESENT &&
          candy[berry][2] == newRow && candy[berry][3] == newCol) {
          // collision
          candyEaten = true;
          break;
        }
      }

      // Move the snake elements by one and possibly increment the snake's length

    }

    if(doRedraw) {
      doRedraw = false;
      textViewClear();

      // Render the playfield
      // First, render the snake itself
      for(byte segment = 0; segment < snakeLength; segment++)
        textViewPutChrAt(segment ? 'Q' : '*', segment ? TV_COLOR_F1H0 : TV_COLOR_F0H1, snake[segment][0], snake[segment][1]);

      // Now render the candy
      for(byte berry = 0; berry < maxCandy; berry++) {
        if(candy[berry][0] == SNAKE_CANDY_PRESENT)
          textViewPutChrAt('$', TV_COLOR_F0H1, candy[berry][2], candy[berry][3]);
      }

      textViewRender();
    }
  }

}

void snakeHighScores(void) {

}
