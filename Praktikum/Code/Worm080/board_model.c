// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//
// The board model

#include <curses.h>
#include <stdlib.h>
#include "worm.h"
#include "board_model.h"
#include "messages.h"

// *************************************************
// Placing and removing items from the game board
// Check boundaries of game board
// *************************************************

enum ResCodes initializeBoard(struct board *aboard) {
  int y;
  // Maximal index of arow, reserve space for message area
  aboard->last_row = LINES - ROWS_RESERVED - 1;
  // Maximal index of a column
  aboard->last_col = COLS - 1;
  
  // Check dimensions of the board
  if (aboard->last_col < MIN_NUMBER_OF_COLS -1 || aboard->last_row < MIN_NUMBER_OF_ROWS - 1) {
    char buf[100];
    sprintf(buf, "Das Fenster ist zu klein: wir brauchen %dx%d", MIN_NUMBER_OF_COLS, MIN_NUMBER_OF_ROWS + ROWS_RESERVED);
    showDialog(buf, "Bitte eine Taste druecken");
    return RES_FAILED;
  }
  // Allocate memory for 2-dimensional array of cells
  // Alloc array of rows 
  printf("@001");
  aboard->cells = (enum BoardCodes **) malloc(LINES * sizeof(enum BoardCodes *));
  if (aboard->cells == NULL) {
    showDialog("Abbruch: Zu wenig Speicher", "Bitte eine Taste druecken");
    return RES_FAILED; // No memory -> direct exit
  }
  printf("@002");
  for (y = 0; y <= LINES; y++) {
    // Allocate array of columns for each y
    printf("@003");
    aboard->cells[y] = (enum BoardCodes *) malloc(COLS * sizeof(enum BoardCodes ));
    if (aboard->cells[y] == NULL) {
      printf("@004");
      showDialog("Abbruch: Zu wenig Speicher", "Bitte eine Taste druecken");
      return RES_FAILED; // No memory -> direct exit
    }
  }
  return RES_OK;
}

void cleanupBoard(struct board* aboard) {
  int y;
  for (y = 0; y <= aboard->last_row; y++) {
    free(aboard->cells[y]);
  }
  free(aboard->cells);
}

// Place an item onto the curses display.
void placeItem(struct board* aboard, int y, int x, enum BoardCodes board_code,  chtype symbol, enum ColorPairs color_pair) {

    //  Store item on the display (symbol code)
    move(y,x);                         // Move cursor to (y,x)
    attron(COLOR_PAIR(color_pair));     // Start writing in selected color
    addch(symbol);         // Store symbol on the virtual display
    attroff(COLOR_PAIR(color_pair));    // Stop writing in selected color
    aboard -> cells[y][x] = board_code;
}

enum ResCodes initializeLevel(struct board* aboard) {
  int i = 5;
  int j = 10;
  int y;
  int x;
  // define local varialbles for loops etc
  // Fill board and screen buffer with empty cells.
  for (y = 0; y <= aboard -> last_row; y++) {
    for (x = 0; x <= aboard -> last_col; x++) {
      placeItem(aboard, y, x, BC_FREE_CELL, SYMBOL_FREE_CELL, COLP_FREE_CELL);
    }
  }
  // Draw a line in order to separate the message area
  // Note: we cannot use function placeItem() since the message area
  // is outside the board!
  for (x = 0; x <= aboard -> last_col; x++) {
    move(y,x);
    attron(COLOR_PAIR(COLP_BARRIER));
    addch(SYMBOL_BARRIER);
    attroff(COLOR_PAIR(COLP_BARRIER));
  }
  // Draw a line to signal the rightmost column of the board
  for (y = 0; y <= aboard -> last_row; y++) {
    placeItem(aboard, y, aboard -> last_col,  BC_BARRIER, SYMBOL_BARRIER, COLP_BARRIER);
  }
  // Barriers use a loop
  for (y = i; y < i + 15; y++) {
    x = aboard -> last_col - (aboard -> last_col / 3);
    placeItem(aboard, y, x, BC_BARRIER, SYMBOL_BARRIER, COLP_BARRIER);
  }
  for (y = j; y <= j + 7; y++) {
    x = aboard -> last_col / 3;
    placeItem(aboard, y, x, BC_BARRIER, SYMBOL_BARRIER, COLP_BARRIER);
  }
  // Food
  // placeItem(aboard, 1, 1, BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
  // placeItem(aboard, 2, 2, BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
 
  // placeItem(aboard, 3, 3, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
  // placeItem(aboard, 4, 2, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
  // placeItem(aboard, 5, 5, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
  // placeItem(aboard, 6, 9, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);

  // placeItem(aboard, 6, 6, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
  // placeItem(aboard, 7, 6, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
  // placeItem(aboard, 9, 3, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
  // placeItem(aboard, 9, 6, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);



  placeItem(aboard, 6, 10, BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);
  placeItem(aboard, 6, 11, BC_FOOD_1, SYMBOL_FOOD_1, COLP_FOOD_1);

  placeItem(aboard, 6, 12, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
  placeItem(aboard, 6, 13, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
  placeItem(aboard, 6, 14, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);
  placeItem(aboard, 6, 15, BC_FOOD_2, SYMBOL_FOOD_2, COLP_FOOD_2);

  placeItem(aboard, 6, 6, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
  placeItem(aboard, 6, 7, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
  placeItem(aboard, 6, 8, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);
  placeItem(aboard, 6, 9, BC_FOOD_3, SYMBOL_FOOD_3, COLP_FOOD_3);


  // Initialize number of food items
  // Attention: must match number of items placed on the board above
  aboard -> food_items = 10;
  return RES_OK;
}

// Getters

// Get the last usable row on the display
int getLastRowOnBoard(struct board* aboard) {
    return aboard -> last_row;
}

// Get the last usable column on the display
int getLastColOnBoard(struct board* aboard) {
    return aboard -> last_col;
}

int getNumberOfFoodItems(struct board* aboard) {
    return aboard -> food_items;
}

enum BoardCodes getContentAt(struct board* aboard, struct pos position){
    return aboard -> cells[position.y][position.x];
}

// Setters
void setNumbersOfFoodItems(struct board* aboard, int n) {
  aboard -> food_items = n;
}

void decrementNumberOfFoodItems(struct board* aboard) {
  aboard -> food_items = aboard -> food_items -1;
}


