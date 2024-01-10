// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "prep.h"
#include "messages.h"
#include "worm.h"
#include "worm_model.h"
#include "board_model.h"
#include "options.h"

// Forward declarations of functions
// ********************************************************************************************

// Management of the game
void initializeColors();
void readUserInput(struct worm* aworm, enum GameStates* agame_state );
enum ResCodes doLevel();

// Management of the game
// ************************************

// Initialize colors of the game
void initializeColors() {
    // Define colors of the game
    start_color();
    init_pair(COLP_USER_WORM, COLOR_GREEN,   COLOR_BLACK);
    init_pair(COLP_FREE_CELL, COLOR_BLACK,   COLOR_BLACK);
    init_pair(COLP_FOOD_1,    COLOR_YELLOW,  COLOR_BLACK);
    init_pair(COLP_FOOD_2,    COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLP_FOOD_3,    COLOR_CYAN,    COLOR_BLACK);
    init_pair(COLP_BARRIER,   COLOR_RED,     COLOR_BLACK);
}

void readUserInput(struct worm* aworm, enum GameStates* agame_state ) {
    int ch; // For storing the key codes

    if ((ch = getch()) > 0) {
        // Is there some user input?
        // Blocking or non-blocking depends of config of getch
        switch(ch) {
            case 'q' :    // User wants to end the show
                *agame_state = WORM_GAME_QUIT;
                break;
            case KEY_UP :// User wants up
                setWormHeading(aworm, WORM_UP);
                break;
            case KEY_DOWN :// User wants down
                setWormHeading(aworm, WORM_DOWN);
                break;
            case KEY_LEFT :// User wants left
                setWormHeading(aworm, WORM_LEFT);
                break;
            case KEY_RIGHT :// User wants right
                setWormHeading(aworm, WORM_RIGHT);
                break;
            case 's' : // User wants single step
                nodelay(stdscr, FALSE);  // We simply make getch blocking
                break;
            case ' ' : // Terminate single step; make getch non-blocking again
                nodelay(stdscr, TRUE);   // Make getch non-blocking again
                break;
            case 'g' : //For development: let the worm grow by BONUS_3 elements
                growWorm(aworm, BONUS_3);
                break;
        }
    }
    return;
}

enum ResCodes doLevel(struct game_options* somegops, enum GameStates* agame_state, char* level_filename) {
    struct worm userworm; // Local variable for storing the user's worm
    struct board theboard;

    enum ResCodes res_code; // Result code from functions
    int end_level_loop;    // Indicates whether we should leave the main loop

    struct pos bottomLeft;  // Start positions of the worm

    // Setup the board
    res_code = initializeBoard(&theboard);
    if (res_code != RES_OK) {
      return res_code;
    }

    // Initialize the current level
    res_code = initializeLevelFromFile(&theboard, level_filename);
    if (res_code != RES_OK) {
      return res_code;
    }

    // There is always an initialized user worm.
    // Initialize the userworm with its size, position, heading.
    bottomLeft.y =  getLastRowOnBoard(&theboard);
    bottomLeft.x =  0;
 
    res_code = initializeWorm(&userworm, (theboard.last_row + 1) * (theboard.last_col + 1), WORM_INITIAL_LENGTH, bottomLeft, WORM_RIGHT, COLP_USER_WORM);
    if ( res_code !=  RES_OK) {
        return res_code;
    }

    // Show worm at its initial position
    showWorm(&theboard, &userworm);
    // Display all what we hev set up until now
    refresh();

    // Start the loop for this level
    end_level_loop = false; // Flag for controlling the main loop
    while(!end_level_loop) {
        // Process optional user input
        readUserInput(&userworm, agame_state); 
        if ( *agame_state == WORM_GAME_QUIT ) {
            end_level_loop = true;
            continue; // Go to beginning of the loop's block and check loop condition
        }

        // Process userworm
        // Clean the tail of the worm
        cleanWormTail(&theboard, &userworm);
        // Now move the worm for one step
        moveWorm(&theboard, &userworm, agame_state);
        
        // Bail out of the loop if something bad happened
        if ( *agame_state !=  WORM_GAME_ONGOING ) {
            end_level_loop = true;
            continue; // Go to beginning of the loop's block and check loop condition
        }
        // Show the worm at its new position
        showWorm(&theboard, &userworm);
        // END process userworm
        
        // Inform user about position and length of userworm in status window
        showStatus(&theboard, &userworm);

        // Sleep a bit before we show the updated window
        napms(somegops->nap_time);

        // Display all the updates
        refresh();

        // Are we done with that level?
        if (getNumberOfFoodItems(&theboard) == 0) {
          end_level_loop = true;
        }

        // Start next iteration
    }

    // Preset res_code for rest of the function
    res_code = RES_OK;

    // For some reason we left the control loop of the current level.
    // Check why according to game_state
    switch (*agame_state) {
      case WORM_GAME_ONGOING:
        if (getNumberOfFoodItems(&theboard) == 0) {
          showDialog("Runde erfolgreich beendet!", "Bitte Taste drücken");
        } else {
          showDialog("Interner Fehler!", "Bitte Taste drücken");
          // correct result code
          res_code = RES_INTERNAL_ERROR;
        }
        break;
      case WORM_GAME_QUIT:
        // User must have typed 'q' for quit
        showDialog("Sie haben die aktuelle Runde abgebrochen!", "Bitte Taste druecken");
        break;
      case WORM_OUT_OF_BOUNDS:
        showDialog("Sie haben das Spiel verloren,"
            " weil Sie das Spielfeld verlassen haben",
            "Bitte Taste druecken");
        break;
      case WORM_CRASH:
        showDialog("Sie haben das Spiel verloren,"
            " weil Sie eine Barriere getroffen haben",
            "Bitte taste druecken");
        break;
      case WORM_CROSSING:
        showDialog("Sie haben das Spiel verloren,"
            " weil Sie einen Wurm gekreuzt haben",
            "Bitte Taste druecken");
        break;
      default:
        showDialog("InternerFehler", "Bitte Taste druecken...");
        res_code = RES_INTERNAL_ERROR;
    }

    // Normal exit point
    
    // remove the worm from display and board
    removeWorm(&theboard, &userworm);
    // Because initialize Worm allocates memory
    cleanupWorm(&userworm);

    cleanupBoard(&theboard);
    return res_code; 
}

enum ResCodes playGame(int argc, char* argv[]) {
  enum ResCodes res_code; // Result code from functions
  enum GameStates game_state; // The current game_state
  // An array of filenames for level descriptions
  // The list must be terminated by a NULL pointer!
  char* level_list[] = {
    "basic.level.1",
    "squaredance.level.2",
    "pirates-doom.level.3",
    NULL
  };
  int cur_level = 0;  // The current level

  struct game_options thegops; // For options passed on the command line

  // Read the command line options
  res_code = readCommandLineOptions(&thegops, argc, argv);
  
  if (res_code != RES_OK) {
    return res_code; // Error: leave early
  }

  if (thegops.start_single_step) {
    nodelay(stdscr, FALSE); // make getch to be a blocking call
  }

  //Play the game
  // At the beginnung of the level, we still have a chance to win
  game_state = WORM_GAME_ONGOING;
  if(thegops.start_level_filename != NULL) {
    // User provided a filename on the command line.
    // Play only this level
    res_code = doLevel(&thegops, &game_state, thegops.start_level_filename);
    
    // From here on we no longer need thegops.start_level_filename
    // Free the memory allocated by strdup in options.c
    free(thegops.start_level_filename);
  
  } else {
    // Play standard level
    while (level_list[cur_level] != NULL && res_code == RES_OK && game_state == WORM_GAME_ONGOING) {
    res_code = doLevel(&thegops, &game_state, level_list[cur_level]);
    if (res_code != RES_OK) {
      return res_code;
    }
    cur_level++;
  }
  if(level_list[cur_level] == NULL && game_state == WORM_GAME_ONGOING) {
     showDialog("Du hast alle Level geschafft!", "Bitte druecke eine Taste");
    } 
  }
  return res_code;
}
// ********************************************************************************************
// MAIN
// ********************************************************************************************

int main(int argc, char* argv[]) {
    getchar();
    enum ResCodes res_code;         // Result code from functions

    // Here we start
    initializeCursesApplication();  // Init various settings of our application
    initializeColors();             // Init colors used in the game

    // Maximal LINES and COLS are set by curses for the current window size.
    // Note: we do not cope with resizing in this simple examples!

    // Check if the window is large enough to display messages in the message area
    // a has space for at least one line for the worm
    if ( LINES < ROWS_RESERVED + MIN_NUMBER_OF_ROWS || COLS < MIN_NUMBER_OF_COLS ) {
        // Since we not even have the space for displaying messages
        // we print a conventional error message via printf after
        // the call of cleanupCursesApp()
        cleanupCursesApp();
        printf("Das Fenster ist zu klein: wir brauchen mindestens %dx%d\n",
                MIN_NUMBER_OF_COLS, MIN_NUMBER_OF_ROWS );
        res_code = RES_FAILED;
    } else {
        res_code = playGame(argc, argv);
        cleanupCursesApp();
    }

    return res_code;
}
