// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011

#include <curses.h>
#include "worm.h"
#include "board_model.h"
#include "worm_model.h"
#include <stdlib.h>
#include "messages.h"

// The worm model
// ********************************************************************************************
// Global variables
// ********************************************************************************************
// *****************************************************
// Functions concerning the management of the worm data
// *****************************************************

// START WORM_DETAIL
// The following functions all depend on the model of the worm

// Initialize the worm
enum ResCodes initializeWorm(struct worm* aworm, int len_max, int len_cur,
    struct pos headpos, enum WormHeading dir, enum ColorPairs color) {
  // Local variables for loops etc.
  int i;

  // Initialize last usable index to len_max -1
  aworm -> maxindex = len_max - 1;
  
  // Current last usable index in array. May grow upto maxindex
  aworm -> cur_lastindex = len_cur - 1;

  //Initialize headindex
  aworm -> headindex = 0;

  // Initialize the array for element positons
  // Allocate an array of the worms length
  aworm -> wormpos = malloc(len_max * sizeof(struct pos));

  if (aworm->wormpos == NULL) {
    showDialog("Abbruch: Zu wenig Speicher", "Bitte eine Taste druecken");
    exit(RES_FAILED); // No memory -> direct exit
  }

  // Mark all elements as unused in the arrays of positions
  // This allows for the effect that the worm appears element by element at the start of each level

  for (i = 0; i <= aworm -> maxindex; i++){
    aworm -> wormpos[i].y = len_cur;
    aworm -> wormpos[i].x = 1;
    }

    // Initialize position of worms head
    aworm -> wormpos[aworm -> headindex] = headpos;

    // Initialize the heading of the worm
    setWormHeading(aworm, dir);

    // Initialze color of the worm
    aworm -> wcolor = color;

    return RES_OK;
}

// Show the worms's elements on the display
// Simple version
extern void showWorm(struct board* aboard, struct worm* aworm) {
    // Due to our encoding we just need to show the head element
    // All other elements are already displayed
    placeItem(
            aboard,
            aworm -> wormpos[aworm -> headindex].y,
            aworm -> wormpos[aworm -> headindex].x,
            BC_USED_BY_WORM,
            SYMBOL_WORM_HEAD_ELEMENT,
            aworm -> wcolor);
    int innerindex;
    innerindex = aworm -> headindex - 1;
    if (innerindex < 0) {
      innerindex = aworm -> cur_lastindex - 1;
    }
    placeItem(
            aboard,
            aworm -> wormpos[innerindex].y,
            aworm -> wormpos[innerindex].x,
            BC_USED_BY_WORM, 
            SYMBOL_WORM_INNER_ELEMENT, 
            aworm -> wcolor);
    int tailindex;
    tailindex = (aworm -> headindex + 1) % aworm -> cur_lastindex;
    if (aworm -> wormpos[tailindex].y != UNUSED_POS_ELEM) {
      placeItem(
              aboard,
              aworm -> wormpos[tailindex].y,
              aworm -> wormpos[tailindex].x,
              BC_USED_BY_WORM,
              SYMBOL_WORM_TAIL_ELEMENT,
              aworm -> wcolor);
    }

}

extern void cleanWormTail(struct board* aboard, struct worm* aworm) {
    // Compute tailindex
    int tailindex = (aworm -> headindex + 1) % aworm -> cur_lastindex;

    // Check the array of worm elements.
    // Is the array element at tailindex already in use?
    // Checking either array theworm_wormpos_y
    // or theworm_wormpos_x is enough.
    if (aworm -> wormpos[tailindex].x != UNUSED_POS_ELEM) {
      // YES: place a SYMBOL_FREE_CELL at the tails position
      placeItem(
              aboard,
              aworm -> wormpos[tailindex].y,
              aworm -> wormpos[tailindex].x,
              BC_FREE_CELL,
              SYMBOL_FREE_CELL,
              COLP_FREE_CELL);
    }
}

extern void moveWorm(struct board* aboard, struct worm* aworm, enum GameStates* agame_state) {
    //struct pos headpos;
    struct pos headpos = aworm -> wormpos[aworm -> headindex];

    // Get the current position of the worm's head element and
    // compute the new head position according to current heading.
    // Do not store the new head position in the array of positions, yet.
    headpos.x = aworm -> wormpos[aworm -> headindex].x;
    headpos.y = aworm -> wormpos[aworm -> headindex].y;

    headpos.x = headpos.x + aworm -> dx;
    headpos.y = headpos.y + aworm -> dy;
    // Check if we would hit something (for good or bad) or are going to leave
    // the display if we move the worm's head according to worm's last
    // direction. We are not allowed to leave the display's window.
    if (headpos.x < 0) {
      *agame_state = WORM_OUT_OF_BOUNDS;
    } else if (headpos.x > getLastColOnBoard(aboard)) {
      *agame_state = WORM_OUT_OF_BOUNDS;
    } else if (headpos.y < 0) {
      *agame_state = WORM_OUT_OF_BOUNDS;
    } else if (headpos.y > getLastRowOnBoard(aboard)) {
      *agame_state = WORM_OUT_OF_BOUNDS;
    } else {
      // We will stay within bounds.
      switch (getContentAt(aboard, headpos)) {
        case BC_FOOD_1:
          *agame_state = WORM_GAME_ONGOING;
          growWorm(aworm, BONUS_1);
          decrementNumberOfFoodItems(aboard);
          break;
        case BC_FOOD_2:
          *agame_state = WORM_GAME_ONGOING;
          growWorm(aworm, BONUS_2);
          decrementNumberOfFoodItems(aboard);
          break;
        case BC_FOOD_3:
          *agame_state = WORM_GAME_ONGOING;
          growWorm(aworm, BONUS_3);
          decrementNumberOfFoodItems(aboard);
          break;
        case BC_BARRIER:
          *agame_state = WORM_CRASH;
          break;
        case BC_USED_BY_WORM:
          *agame_state = WORM_CROSSING;
          break;
        default: {}
      }
    }

      if (*agame_state == WORM_GAME_ONGOING) {
        aworm -> headindex = (aworm -> headindex + 1) % aworm -> cur_lastindex;
        if (aworm -> headindex >= aworm -> cur_lastindex)  {
          aworm -> headindex = 0;
       }
      // Store new coordinates of head element in worm structure
      aworm -> wormpos[aworm -> headindex] = headpos;
    }
}

void growWorm(struct worm* aworm, enum Boni growth) {
  if (aworm -> cur_lastindex + growth <= aworm -> maxindex) {
    aworm -> cur_lastindex += growth;
  } else {
    aworm -> cur_lastindex = aworm -> maxindex;
  }
}

// Getters
struct pos getWormHeadPos(struct worm* aworm){
  // Structures are passed by value!
  // -> we return a copy here
  return aworm -> wormpos[aworm -> headindex];
}

int getWormLength(struct worm* aworm){
  return aworm -> cur_lastindex;
}

// Setters
extern void setWormHeading(struct worm* aworm, enum WormHeading dir) {
    switch(dir) {
        case WORM_UP :// User wants up
            aworm -> dx=0;
            aworm -> dy=-1;
            break;
        case WORM_DOWN :// User wants down
            aworm -> dx=0;
            aworm -> dy=1;
            break;
        case WORM_LEFT :// User wants left
            aworm -> dx=-1;
            aworm -> dy=0;
            break;
        case WORM_RIGHT :// User wants right
            aworm -> dx=1;
            aworm -> dy=0;
            break;
    }
}

void cleanupWorm(struct worm* aworm) {
  // free array of wormpos
  free(aworm->wormpos);
}

// Remove a worm from the board and clean the display
void removeWorm(struct board* aboard, struct worm* aworm) {
  int i;
  i = aworm -> headindex;
  do {
    placeItem(aboard, aworm -> wormpos[i].y, aworm -> wormpos[i].x, BC_FREE_CELL, SYMBOL_FREE_CELL, COLP_FREE_CELL);
    // Advance index; go round after aworm -> cur_lastindex
    i = (i - 1);
  } while (i != 0);
}

// END WORM_DETAIL
// ********************************************************************************************
