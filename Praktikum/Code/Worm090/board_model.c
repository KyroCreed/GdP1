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
#include <string.h>
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
 
  aboard->cells = (enum BoardCodes **) malloc(LINES * sizeof(enum BoardCodes *));
  if (aboard->cells == NULL) {
    showDialog("Abbruch: Zu wenig Speicher", "Bitte eine Taste druecken");
    return RES_FAILED; // No memory -> direct exit
  }
   for (y = 0; y <= aboard->last_row; y++) {
    // Allocate array of columns for each y
      aboard->cells[y] = (enum BoardCodes *) malloc(COLS * sizeof(enum BoardCodes ));
    if (aboard->cells[y] == NULL) {
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


// Read level decription from file
// We allow for level descriptions of dimensions
//    (aboard->last_row + 1) x (last_col + 1)
enum ResCodes initializeLevelFromFile(struct board* aboard, const char* filename) {
    int y,x;
    int rownr;
    char buf[100];  // for messages
    FILE* in;       // FILE pointer for reading from file

    // Fill board with empty cells.
    for (y = 0; y <= aboard->last_row; y++) {
        for (x = 0; x <= aboard->last_col; x++) {
            placeItem(aboard, y, x, BC_FREE_CELL, SYMBOL_FREE_CELL, COLP_FREE_CELL);
        } 
    }

    // Initialize food_items
    aboard->food_items = 0;

    // Read at most aboard->last_row+1 lines from file
    // Read at most aboard->last_col+1 characters per line from file
    // --> We need a buffer of size (aboard->last_col+1 + 2 )
    // The additional 2 elements are for '\n' and '\0'
    int bufsize = aboard->last_col + 3;
    char* buffer;
    if ((buffer = malloc(sizeof(char) * bufsize)) == NULL) {
        sprintf(buf,"Kein Speicher mehr in initializeLevelFromFile\n");
        showDialog(buf,"Bitte eine Taste druecken");
        return RES_FAILED;
    }

    // Open the file
    if ( (in = fopen(filename,"r")) == NULL) {
        sprintf(buf,"Kann Datei %s nicht oeffnen",filename);
        showDialog(buf,"Bitte eine Taste druecken");
        return RES_FAILED;
    }

    rownr = 0;
    // Read all lines from the text file describing the level
    while (rownr < aboard->last_row+1 && ! feof(in)) {
        int len;
        // Read one line from file
        if (fgets(buffer,bufsize,in) != NULL) {
            // The specifiction of fgets guarantees that there are
            // at least two characters in the buffer, namely '\n' followed by '\0'.
            // Exception: buffer was full before '\n' was read. Then we only
            // have a '\0' in the buffer.
            len = strlen(buffer);
            // Note: function strlen does not count the terminating '\0'.
            if (buffer[len -1] != '\n') {
                // Input line was not yet finished. No '\n' in buffer.
                // Delete the last char before '\0'from the buffer because
                // it exceeds the allowed number of characters.
                buffer[len -1] = '\0';
                // Delete the rest of the line that is already in OS input buffer
                while (fgetc(in) != '\n'){;}
            } else {
                // Input line in buffer ends with '\n'
                // Delete the '\n' from the buffer
                buffer[len -1] = '\0';
            }
        } else {
            // fgets returned NULL
            // The reason may be either End Of File (EOF) or a real read error.
            // We immediately return on real read error
            if (!feof(in)) {
                char buf[100];
                sprintf(buf,"Fehler beim Lesen von Zeile %d aus Datei %s",
                        rownr +1,filename);
                showDialog(buf,"Bitte eine Taste druecken");
                return RES_FAILED;
            } else {
                // We got EOF, skip rest of loop
                // Loop condition will fire
                continue;
            }
        }
        // If we reach this line, our buffer is filled with the current input line.
        // Due to our logic above the variable len is properly set.
        // However, for the sake of clarity we compute the length again
        len = strlen(buffer);

        // Fill the board's row with the symbols specified in the current input line
        for (x = 0; x <= aboard->last_col && x < len; x++) {
            switch (buffer[x]) {
                case SYMBOL_BARRIER:
                    placeItem(aboard,rownr,x,BC_BARRIER,SYMBOL_BARRIER,COLP_BARRIER);
                    break;
                case SYMBOL_FOOD_1:
                    placeItem(aboard,rownr,x,BC_FOOD_1,SYMBOL_FOOD_1,COLP_FOOD_1);
                    aboard->food_items++;
                    break;
                case SYMBOL_FOOD_2:
                    placeItem(aboard,rownr,x,BC_FOOD_2,SYMBOL_FOOD_2,COLP_FOOD_2);
                    aboard->food_items++;
                    break;
                case SYMBOL_FOOD_3:
                    placeItem(aboard,rownr,x,BC_FOOD_3,SYMBOL_FOOD_3,COLP_FOOD_3);
                    aboard->food_items++;
                    break;

                // We ignore all other symbols! 
            }
        }
        // advance to next input line
        rownr++;
    } // END while
    
    // Free the line buffer
    free(buffer);

    // Draw a line in order to separate the message area
    // Note:
    // we cannot use function placeItem() since the message area is outside the board!
    y = aboard->last_row + 1;
    for (x=0; x <= aboard->last_col; x++) {
        move(y,x);
        attron(COLOR_PAIR(COLP_BARRIER));
        addch(SYMBOL_BARRIER);
        attroff(COLOR_PAIR(COLP_BARRIER));
    }

    fclose(in);
    return RES_OK;
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


