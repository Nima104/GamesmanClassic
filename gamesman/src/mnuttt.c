// $id$
// $log$

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mnuttt.c
**
** DESCRIPTION: Nu Tic-Tac-Toe
**
** AUTHOR:      Guy Boo, Ming (Evan) Huang
**
** DATE:        Initial editing started on 2/5/05.
**              Ending date: unknown yet
**
** UPDATE HIST: legend: [+]: feature/new stuff,
**                      [-]: old stuff removed/deprecated,
**                      [*]comments
**
**              2/5/05: [+] initial commit, some game-specific consts,
**                          global variables for directions, IntitializeGame(),
**                          PrintPosition(), DoMove(), primitive(),
**                          printComputersMove (), Hash/unhash for moves,
**                          and other helpers.
**                      [*] we may need to consult Garcia to see of our board
**                          is too big.  see the note in gNumberOfPositions
**                          global variables for directions,
**                          IntitializeGame(), PrintPosition(), DoMove(),
**                          primitive(), printComputersMove (), Hash/unhash for
**                          moves, and other helpers.
**              2/6/05: [+] fixed some hardcoded constants, Position(), Row(),
**                          Column() now operates on an arbitrarily sized
**                          board.  Easy job for getAndPrintUserInput().
**              2/7/05: [+] Added validTextMove(), convertTextInputToMove()
**                      [*] Game should be playable by now. Added rules to
**                          Makefile.  GenerateMoves () is next.
**              2/8/05: [+] GenerateMove () complete, borrowing Guys's code.
**              2/9/05: [*] Corrected some indexing and pointer problems.
**                      [+] Implemented GetAndPrintPlayersMove correctly and
**                          hotwired GetInitialPosition
**              2/11/05 [*] Guy: added function prototypes so it compiles on
**                          my backwards compiler, and revised getOptions so it
**                          returns stuff that makes sense.  hope i didn't
**                          break it!
**              2/14/05 [+] Merged changes from Guy's code, as instructed.
**                          Various abstraction fixes, more prototypes, new
**                          func setGameParameters()
**              2/15/05 [+] Added diagonal movement and redefined input formats
**                          for it.  GenerateMoves() takes care of it.
**                          variants of the game now include variable board,
**                          variable number of pieces in a line for victory,
**                          misere play, and diagonal moves.
**
**************************************************************************/


/* FORMATS:
 * POSITIONs that come out of here must encode the entire game state, including
 * the board layout and whose turn it is.  so if i move my x one space down, i
 * return a position representing that and the fact that it's o's turn.
 * MOVEs need to encode anything having to do with a player's move, but note
 * that they do not (necessarily) encode who performs it.
 *
 * from studying mjoust.c, it appears that both formats are arbitrary and left
 * entirely up to us. note, though, that the definition implies that
 * maintaining state is futile - we're only defining the rulepacks.
 * redefinition of MOVE:
 *     fx,fy,tx,ty - ie use different base combination as discussed at last
 *                   meeting. this is for undo compatibility, and to not
 *                   restrict boardsize or move types.*/


/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Nu Tic-Tac-Toe";
STRING   kAuthorName          = "Guy Boo and Ming (Evan) Huang";
STRING   kDBName              = "nuttt"; /* The name of the stored database */

BOOLEAN  kPartizan            = FALSE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = FALSE ;
BOOLEAN  kTieIsPossible       = FALSE ;
BOOLEAN  kLoopy               = TRUE ;
/* the following two vars should be TRUE only during debugging. */
BOOLEAN  kDebugMenu           = TRUE ;
BOOLEAN  kDebugDetermineValue = TRUE ;

/* as we discussed, the total number of positions is 20!/(12!*4!*4!), which
 * equals 8,817,900.  this will be fine for generic hash.
 */
POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
/* we cannot reliably calculate this parameter, so we rely on hash_init() */
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

/* 
 * Help strings that are pretty self-explanatory
 * Strings that span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface = ""; /* kSupportsGraphics == FALSE */
STRING   kHelpTextInterface =
  "On your turn, enter the x and y coordinates of the piece you'd like to move\n\
and the direction you wish to move it. Enter your move in the format\n\
<column> <row> <direction> where the direction is one of the directions from 5 on\n\
the keypad, i.e. 8 for up. 5 will not be accepted. For example, to move the piece \n\
in column 3 and row 2 up one square, you would enter '3 2 8'.";

STRING   kHelpOnYourTurn =
"You should type: <row number> <column number> <direction>, direction is one of \n\
1 2 3 4 6 7 8 9";

STRING   kHelpStandardObjective =
  "Get any three of your pieces to form a horizontal, diagonal, or vertical\n\
line.";

STRING   kHelpReverseObjective =
  "Force your opponent to arrange three pieces in a horizontal, diagonal,\n\
or vertical line.";

STRING   kHelpTieOccursWhen = ""; /* kTieIsPossible == FALSE */

STRING   kHelpExample = "coming soon!";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/
#define PLAYER1_PIECE       'X'
#define PLAYER2_PIECE       'O'
#define EMPTY_PIECE         ' '
#define MOVE_FORMAT        "%d%d %c%c\n"
#define PLAYER1_TURN        1
#define PLAYER2_TURN        2
#define INPUT_PARAM_COUNT1  3
#define INPUT_PARAM_COUNT2  4
#define BOARD_SIZE          (BOARD_ROWS*BOARD_COLS)
#define PLAYER_PIECES       BOARD_COLS
#define EMPTY_PIECES        ((BOARD_ROWS-2)*BOARD_COLS)
#define NUM_OF_DIRS         9
#define BOARD_DIM_CAP       10
#define NUM_TO_WIN_CAP      8

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int     BOARD_ROWS          = 5;
int     BOARD_COLS          = 4;
int     NUM_TO_WIN          = 3;
BOOLEAN CAN_MOVE_DIAGONALLY = FALSE;
BOOLEAN MISERE              = FALSE;

/*the corresponding direction for output*/
STRING directions[NUM_OF_DIRS] = {
  "SW", "S", "SE",
  "W",  "",  "E",
  "NW", "N",   "NE"
};

char* alpha = "abcdefghij"; /* used to print row letter */

/*the increments to the row and column numbers of the piece *
* THESE ARE ACCORIDNG TO THE KEYPAD ARRANGEMENTS
   7  8  9
   4  5  6
   1  2  3*/
int dir_increments[NUM_OF_DIRS][2] = {
        { 1, -1 } , { 1 , 0 } , { 1 , 1 } , 
	{ 0, -1 } , { 0 , 0 } , { 0 , 1 } ,
	{ -1, -1} , { -1, 0 } , { -1, 1 }
};

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();
extern int              generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg));
extern int              generic_hash(char *board, int player);
extern char            *generic_unhash(int hash_number, char *empty_board);
extern int              whoseMove (int hashed);
/*internal*/
void                    InitializeGame();
MOVELIST               *GenerateMoves(POSITION position);
POSITION                DoMove (POSITION position, MOVE move);
VALUE                   Primitive (POSITION position);
void                    PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn);
void                    printBoard(char board[]);
void                    PrintComputersMove(MOVE computersMove, STRING computersName);
void                    PrintMove(MOVE move);
USERINPUT               GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName);
BOOLEAN                 ValidTextInput(STRING input);
MOVE                    ConvertTextInputToMove(STRING input);
void                    GameSpecificMenu();
void                    SetTclCGameSpecificOptions(int options[]);
POSITION                GetInitialPosition();
BOOLEAN                 GetInitPosHelper (char *board, char playerchar);
int                     NumberOfOptions();
int                     getOption();
void                    setOption(int option);
void                    DebugMenu();
/* helpers */
int                     Index(int row, int col);
int                     Row(int Pos);
int                     Column(int Pos);
MOVE                    Hasher(int row, int col, int dir);
int                     Unhasher_Index(MOVE move);
int                     Unhasher_Direction(MOVE move);
void                    setGameParameters(int row, int col, int num_of_dirs, BOOLEAN diag, BOOLEAN misere);
void                    initializePiecesArray(int p_a[]);
void                    initializeBoard(char board[]);

/*************************************************************************
**
** Global Database Declaration
**
**************************************************************************/

extern VALUE     *gDatabase;


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**              Sets up gDatabase (if necessary).
** 
************************************************************************/

void InitializeGame ()
{
    char board[BOARD_SIZE];
    int init_pieces[10];

    setGameParameters (5, 4, 3, FALSE, FALSE);  /*default values*/
    initializePiecesArray(init_pieces);
    initializeBoard(board);
    gNumberOfPositions = generic_hash_init (BOARD_SIZE, init_pieces, NULL);
    gInitialPosition = generic_hash(board, PLAYER1_TURN);
}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : Current position for move
**                                  generation.
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{
  MOVELIST *moves = NULL;
  int i, j, k, di, dj;
  char board[BOARD_SIZE];
  int player = whoseMove(position);
  BOOLEAN canProcessDir = FALSE;

  generic_unhash (position, board);
  /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
  for (i = 0; i < BOARD_ROWS; i++) {
    for (j = 0; j < BOARD_COLS; j++) {
      if (((player==1) && (board[Index(i,j)] == PLAYER1_PIECE)) ||
	  ((player==2) && (board[Index(i,j)] == PLAYER2_PIECE)))
      {
	for (k = 0; k < NUM_OF_DIRS; k++) {
	  // for every possible movement as specified in dir_increments, except 5, and the diagonals when !CAN_MOVE_DIAGONALLY
	  if (k == 4)
	    canProcessDir = FALSE;
	  else if ((k == 0) || (k == 2) || (k == 6) || (k == 8))
	      canProcessDir = CAN_MOVE_DIAGONALLY;
	  else
	    canProcessDir = TRUE;
	  if (canProcessDir) {
	    di = i + dir_increments[k][0];
	    dj = j + dir_increments[k][1];
	    if ((di >= 0) && (dj >= 0) &&
		(di < BOARD_ROWS) && (dj < BOARD_COLS) &&
		(board[Index(di,dj)] == EMPTY_PIECE)) {
	      moves = CreateMovelistNode(Hasher(i,j,k),moves);
	    }
	  }
	}
      }
    }
  }    
  return moves;
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
** 
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       Some Board Hash Function
**              Some Board Unhash Function
**
*************************************************************************/

POSITION DoMove (POSITION position, MOVE move)
{
       char board[BOARD_SIZE];
       int current_player = whoseMove(position);
       int move_position = Unhasher_Index(move);
       int row = Row (move_position);
       int col = Column (move_position);
       int direction = Unhasher_Direction(move);
       int new_position = Index(row + dir_increments[direction][0], col + dir_increments[direction][1]);

       generic_unhash (position, board);
       board[new_position] = board[move_position];
       board[move_position] = (char) EMPTY_PIECE;
       return generic_hash(board, (current_player == PLAYER1_TURN ? PLAYER2_TURN : PLAYER1_TURN));
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              Example: Tic-tac-toe - Last piece already placed
**
**              Case                                  Return Value
**              *********************************************************
**              Current player sees three in a row    lose
**              Entire board filled                   tie
**              All other cases                       undecided
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE)           : one of
**                                  (win, lose, tie, undecided)
**
** CALLS:       None              
**
************************************************************************/

VALUE Primitive (POSITION position)
{
  /* if it's your turn but there's already a win, then you must have lost!
   * can think about it this way because players are only allowed to move their
   * own pieces.
   */
  char board[BOARD_SIZE];
  int x,y,d;
  
  generic_unhash(position, board);
  
  for (y = 0; y < BOARD_ROWS; y++) /*horizontal*/
    for (x = 0; x < BOARD_COLS+1-NUM_TO_WIN; x++)
      if (board[Index(y,x)] != EMPTY_PIECE) {
	for (d = 1; d < NUM_TO_WIN; d++)
	  if (board[Index(y,x+d-1)] != board[Index(y,x+d)])
	    break;
	if (d == NUM_TO_WIN) return (MISERE ? win : lose);
      }
  
  for (y = 0; y < BOARD_ROWS+1-NUM_TO_WIN; y++) /*vertical*/
    for (x = 0; x < BOARD_COLS; x++)
      if (board[Index(y,x)] != EMPTY_PIECE) {
	for (d = 1; d < NUM_TO_WIN; d++)
	  if (board[Index(y+d-1,x)] != board[Index(y+d,x)])
	    break;
	if (d == NUM_TO_WIN) return (MISERE ? win : lose);
      }
  
  for (y = 0; y < BOARD_ROWS+1-NUM_TO_WIN; y++) { /*diagonal*/
    for (x = 0; x < BOARD_COLS+1-NUM_TO_WIN; x++) {
      if (board[Index(y,x)] != EMPTY_PIECE) {
	for (d = 1; d < NUM_TO_WIN; d++)
	  if (board[Index(y+d-1,x+d-1)] != board[Index(y+d,x+d)])
	    break;
	if (d == NUM_TO_WIN) return (MISERE ? win : lose);
      }
      
      if (board[Index(y+NUM_TO_WIN-1,x)] != EMPTY_PIECE) {
	for (d = 1; d < NUM_TO_WIN; d++)
	  if (board[Index(y+NUM_TO_WIN-d,x+d-1)] != board[Index(y+NUM_TO_WIN-d-1,x+d)])
	    break;
	if (d == NUM_TO_WIN) return (MISERE ? win : lose);
      }
    }
  }
  
  return undecided;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
** 
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()      : Returns the prediction of the game
**
************************************************************************/

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
    char board[BOARD_SIZE];

    generic_unhash (position, board);
    printf (" The game board as %s sees it:\n", playersName);
    printBoard (board);
}

void printBoard (char *board) {
    int i,j;

    for (i = 0; i < BOARD_ROWS; i++) {
      printf ("  +");
      for (j = 0; j < BOARD_COLS; j++)
	printf ("-+");
      printf ( "\n%c " , alpha[i] );
      for (j = 0; j < BOARD_COLS; j++) 
	printf ("|%c", board[Index(i,j)]);
      printf("|\n");
    }
    printf("  +");
    for (j = 0; j < BOARD_COLS; j++) {
      printf("-+");
    printf("\n   ");
    for (j = 0; j < BOARD_COLS; j++) {
      printf("%d ", j);
    }
    printf("\n");
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
** 
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName)
{
    int position = Unhasher_Index(computersMove);
    int direction = Unhasher_Direction(computersMove);

    printf ("%s moves the piece %c%d %s\n", computersName, \
	    alpha[Row(position)], Column (position), directions[direction]);
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
** 
** INPUTS:      MOVE move         : The move to print. 
**
************************************************************************/

void PrintMove (MOVE move)
{
    int position = Unhasher_Index(move);
    int direction = Unhasher_Direction(move);

    printf ("%c%d %s\n", alpha[Row (position)], Column (position), \
	    directions[direction]);
}


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here. 
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move. 
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
    USERINPUT input;
    USERINPUT HandleDefaultTextInput();
    
    for (;;) {
        /***********************************************************
         * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
         ***********************************************************/
	printf("%8s's move [(undo)/<row> <col> <dir>] : ", playersName);
	
	input = HandleDefaultTextInput(position, move, playersName);
	
	if (input != Continue)
		return input;
    }

    /* NOTREACHED */
    return Continue;
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER
**                                          COMMANDS IN YOUR GAME
**              ?, s, u, r, h, a, c, q
**                                          However, something like a3
**                                          is okay.
** 
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9
**                       Only integers between 1 to 9 are accepted
**                       regardless of board position.
**                       Moves will be checked by the core.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
    int result, i, j;
    char dir1 = '\0';
    char dir2 = '\0';
    result = sscanf (input, MOVE_FORMAT, &i, &j, &dir1, &dir2);
    if ((result != INPUT_PARAM_COUNT1) && (result != INPUT_PARAM_COUNT2))
      return FALSE;
    if ((i >= 0) && (j >= 0) && (i < BOARD_ROWS) && (j < BOARD_COLS) &&
	( ((dir1=='E') || (dir1=='W')) || ( ((dir1=='N')||(dir1=='S')) &&
					    ((dir2=='E')||(dir2=='W')) ))) {
      return TRUE;
    } else {
      return FALSE;
    }
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**              Gamesman already checked the move with ValidTextInput
**              and ValidMove.
** 
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
  int row, col, dir;
  char dir1 = '\0';
  char dir2 = '\0';
  sscanf (input, MOVE_FORMAT, &row, &col, &dir1, &dir2);
  if (dir1 == 'E') {
    dir = 5;
  } else if (dir1 == 'W') {
    dir = 3;
  } else if (dir1 == 'N') {
    if (dir2 == 'E') {
      dir = 8;
    } else if (dir2 == 'W') {
      dir = 6;
    } else {
      dir = 7;
    }
  } else if (dir1 == 'S') {
    if (dir2 == 'E') {
      dir = 2;
    } else if (dir2 == 'W') {
      dir = 0;
    } else {
      dir = 1;
    }
  } else {
    printf("ERROR - mnuttt.c line 654 - got a bad dir1: %c", dir1);
  }
  return Hasher (row, col, dir);
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
**
**              Examples
**              Board Size, Board Type
**
**              If kGameSpecificMenu == FALSE
**                   Gamesman will not enable GameSpecificMenu
**                   Gamesman will not call this function
** 
**              Resets gNumberOfPositions if necessary
**
************************************************************************/

void GameSpecificMenu ()
{
    
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions (int options[])
{
    
}
  
  
/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Called when the user wishes to change the initial
**              position. Asks the user for an initial position.
**              Sets new user defined gInitialPosition and resets
**              gNumberOfPositions if necessary
** 
** OUTPUTS:     POSITION : New Initial Position
**
************************************************************************/

POSITION GetInitialPosition ()
{
  char board[BOARD_SIZE];
  int i;
  const int player = 1;
  char input[80];
  BOOLEAN setboard = TRUE;
  BOOLEAN tryagain = TRUE;

  while (tryagain) {
    for (i = 0; i < BOARD_SIZE; i++) {
      board[i] = (char) EMPTY_PIECE;
    }
    printf("please enter board locations for the pieces in 'x y' format.\n");
    printf("for example, to place a piece at (0,1) you'd enter '0 1'.\n");
    printf("enter 'quit' or 'exit' to leave this function and return to\n");
    printf("the default initial board.\n");
    if (!(setboard = tryagain = GetInitPosHelper(board, PLAYER1_PIECE))) break;
    if (!(setboard = tryagain = GetInitPosHelper(board, PLAYER2_PIECE))) break;
    while (TRUE) {
      printf("is this the board you want? (y/n)");
      printBoard(board);
      gets(input);
      if ((strcmp(input,"y") == 0) || (strcmp(input,"yes") == 0)) {
	setboard = TRUE;
	tryagain = FALSE;
	break;
      } else if ((strcmp(input,"n") == 0) || (strcmp(input,"no") == 0)) {
	setboard = FALSE;
	tryagain = TRUE;
	break;
      } else {
	printf("error. enter 'y', 'yes', 'n', or 'no'.\n");
      }
    }
  }
  if (setboard) gInitialPosition = generic_hash(board, player);
  return gInitialPosition;
}

BOOLEAN GetInitPosHelper (char *board, char playerchar) {
  int i,x,y;
  char input[80];
  for (i = 0; i < PLAYER_PIECES; i++) {
    printf("enter the location of %c piece %d: ",playerchar,i+1);
    gets(input);
    if ((strcmp(input,"quit") == 0) || (strcmp(input,"exit") == 0)) {
      return FALSE;
    } else {
      sscanf(input, "%d %d", &x, &y);
      if ((x < 0) || (y < 0) || (x >= BOARD_COLS) || (y >= BOARD_ROWS)) {
	printf("error - off board.");
	i--;
	continue;
      } else if (board[Index(y,x)] != EMPTY_PIECE) {
	printf("error - already occupied.");
	i--;
	continue;
      } else {
	board[Index(y,x)] = playerchar;
      }
    }
  }
  return TRUE;
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of variants
**              your game supports.
**
** OUTPUTS:     int : Number of Game Variants
**
************************************************************************/

int NumberOfOptions ()
{
    return 4;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function that returns a number corresponding
**              to the current variant of the game.
**              Each set of variants needs to have a different number.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption ()
{
  int result = BOARD_ROWS;
  result = (result * BOARD_DIM_CAP) + BOARD_COLS;
  result = (result * NUM_TO_WIN_CAP) + NUM_TO_WIN;
  result = result << 2;
  if (CAN_MOVE_DIAGONALLY) result += 2;
  if (MISERE) result += 1;
  return result;
 }


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash function for game variants.
**              Unhashes option and sets the necessary variants.
**
** INPUT:       int option : the number representation of the options.
**
************************************************************************/

void setOption (int option)
{
  int row, col, num_to_win;
  BOOLEAN diag, misere;

  misere = option % 2;
  diag = option / 2 % 2;
  option /= 4;
  num_to_win = option % NUM_TO_WIN_CAP;
  option /= NUM_TO_WIN_CAP;
  col = option % BOARD_DIM_CAP;
  option /= BOARD_DIM_CAP;
  row = option;
  setGameParameters (row, col, num_to_win, diag, misere);
  InitializeGame();
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default
**              debug menu). Menu used to debug internal problems.
**
**              If kDebugMenu == FALSE
**                   Gamesman will not display a debug menu option
**                   Gamesman will not call this function
** 
************************************************************************/

void DebugMenu ()
{
    
}


/************************************************************************
**
** Everything specific to this module goes below these lines.
**
** Things you want down here:
** Move Hasher
** Move Unhasher
** Any other function you deem necessary to help the ones above.
** 
************************************************************************/

int Index(int row, int col) {
  return  (row*BOARD_COLS+col);
}

int Row(int Pos) {
  return (Pos/BOARD_COLS);
}

int Column(int Pos) {
  return (Pos%BOARD_COLS);
}

MOVE Hasher(int row, int col, int dir) {
  return (MOVE) ( ( Index(row, col) )* NUM_OF_DIRS + dir);
}

int Unhasher_Index(MOVE hashed_move) {
  return (hashed_move / NUM_OF_DIRS);
}

int Unhasher_Direction(MOVE hashed_move) {
  return (hashed_move % NUM_OF_DIRS);
}

void setGameParameters (int row, int col, int num_to_win, BOOLEAN diag, BOOLEAN misere) {
  BOARD_ROWS = row;
  BOARD_COLS = col;
  NUM_TO_WIN = num_to_win;
  CAN_MOVE_DIAGONALLY = diag;
  MISERE = misere;
}

void initializePiecesArray(int p_a[]) {
  p_a[0] = PLAYER1_PIECE;
  p_a[3] = PLAYER2_PIECE;
  p_a[6] = EMPTY_PIECE;
  p_a[9] = -1;
  p_a[1] = p_a[2] = p_a[4] = p_a[5] = PLAYER_PIECES;
  p_a[7] = p_a[8] = EMPTY_PIECES;
}

void initializeBoard (char board[]) {
  int x,y;
  char piece;

  for (x = 0; x < BOARD_COLS; x++) {
    for (y = 0; y < BOARD_ROWS; y++) {
      if (y == 0) {
	piece = (x % 2) ? PLAYER2_PIECE : PLAYER1_PIECE;
      } else if (y == BOARD_ROWS-1) {
	piece = (x % 2) ? PLAYER1_PIECE : PLAYER2_PIECE;
      } else {
	piece = EMPTY_PIECE;
      }
      board[Index(y, x)] = (char) piece;
    }
  }
}
