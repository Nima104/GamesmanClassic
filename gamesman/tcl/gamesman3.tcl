############################################################################
##
## Source The Utility Functions
##
##
##
#############################################################################

proc stack {} {
    return [list]
}

proc push {l a} {
    return [linsert $l 0 $a]
}

proc pop {l} {
    return [lreplace $l 0 0]
}

proc peek {l} {
    return [lindex $l 0]
}

## returns whether or not the list l contains the key a
proc containskey {a l} {
    set retval false
    foreach item $l {
        if {[lindex $item 0] == $a} {  
            set retval true
        } 
    }
    return $retval
}

##### other utility function

proc MoveValueToColor { moveType value } {
    set color cyan
    if {$moveType == "value"} {
	if {$value == "Tie"} {
	    set color yellow
	} elseif {$value == "Lose"} {
	    set color green
	} else {
	    set color red4
	}
    } elseif {$moveType == "all"} {
	set color cyan
    }
    return $color
}


#############################################################################
##
## InitConstants
##
## This procedure is used to reserve some keywords (which always begin with
## 'k' to remember they're constants to make the code easier to read. 
## Constants never change. Nice to know somethings don't change around here.
##
#############################################################################

proc InitConstants {} {

    ### These are used in the routine that draws the pieces - we want a small
    ### piece used as a label but a larger piece used in the playing board.

    global kBigPiece
    set kBigPiece 1
    
    global kSmallPiece
    set kSmallPiece 0

    ### Set the color and font of the labels

    global kLabelFont
    set kLabelFont { Helvetica 12 bold }

    global kLabelColor
    set kLabelColor grey40

    ### Set the animation speed

    global gAnimationSpeed
    set gAnimationSpeed 8
    
    global gPredictions
    set gPredictions true
    
    global gMoveType
    set gMoveType all
    
    global gMoveRadioStation
    set gMoveRadioStation all

    global gGameSolved 
    set gGameSolved false

    set rules [GS_GetGameSpecificOptions]

    set kStandardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]
    global gRuleset
    set gRuleset [linsert $rules 0 $kStandardRule]
    
    global gNumberOfRules
    set gNumberOfRules [llength $gRuleset]

    for { set i 0 } { $i < $gNumberOfRules } { incr i } {
	global gRule$i
	set gRule$i 0
    }

    eval [concat C_SetGameSpecificOptions [GetAllGameOptions]]
}

#############################################################################
##
## InitWindow
##
## Here we initialize the window. Set up fonts, colors, widgets, helps, etc.
##
#############################################################################

proc InitWindow {} {
 
    global kGameName
    global kLabelFont
    
    wm title    . "GAMESMAN $kGameName v2.0 (1999-05-01)"
    wm geometry . "+10+10"
    global tcl_platform
    if { $tcl_platform(platform) == "macintosh" || \
         $tcl_platform(platform) == "windows" } {
        console hide
    }
    
    
    ### f1 = The Setup frame (Setup: About,Start,Rules,Help,Quit)
    
    frame .f1
    button .f1.butStart \
        -text "Start" \
        -font $kLabelFont \
        -command DoStart

    ### Pack everything in
    
    pack append .f1 \
        .f1.butStart   {left pady 10 expand fill }
    
    pack append . \
        .f1 {top expand fill}
}


proc SetupGamePieces {} {

    global gLeftPiece gRightPiece gLeftHumanOrComputer gRightHumanOrComputer
    
    global gLeftName gRightName
    global gPiecesPlayersName
    set alist [GS_NameOfPieces]
    
    set gLeftPiece [lindex $alist 0]
    set gRightPiece [lindex $alist 1]
    set gLeftHumanOrComputer Human
    set gRightHumanOrComputer Human
    set gLeftName Player
    set gRightName Hal9000
    
    set gPiecesPlayersName($gLeftPiece) $gLeftName
    set gPiecesPlayersName($gRightPiece) $gRightName

    global gLeftColor gRightColor
    set alist [GS_ColorOfPlayers]
    set gLeftColor [lindex $alist 0]
    set gRightColor [lindex $alist 1]

}



#############################################################################
##
## New Game
##
## This is what we do when the user clicks the 'New Game' button.
##
## Here we set up the globals for the new game and call the game-specific 
## function GS_NewGame to draw the initial screen
##
## Args: None
## 
## Requires: GS_Initialize has been called
##
#############################################################################

proc NewGame { } {

    global gGameSoFar gPosition gInitialPosition gMovesSoFar
    global gLeftName gRightName gWhoseTurn
    .middle.f1.cMLeft itemconfigure LeftName \
	-text [format "Player1:\n%s" $gLeftName]
    .middle.f3.cMRight itemconfigure RightName \
	-text [format "Player2:\n%s" $gRightName]	
    .middle.f1.cMLeft raise LeftName
    .middle.f3.cMRight raise RightName
    update
    set gPosition $gInitialPosition
    set gWhoseTurn "Left"
    set gGameSoFar [list $gInitialPosition]
    set gMovesSoFar [list]
    GS_NewGame .middle.f2.cMain $gPosition
    EnableMoves
    DriverLoop
}

#############################################################################
##
## DriverLoop
##
## Here is where we decide whether to get a move from the database or give
## up control to the user and allow for a human move.  
##
## Args: None
##
## Requires: New Game has been called 
##
#############################################################################

proc DriverLoop { } {
   
    ## retrieve global variables
    global gGameSoFar gMovesSoFar gPosition gWaitingForHuman

    set gWaitingForHuman false

    while { [expr !$gWaitingForHuman] } {

	set primitive [C_Primitive $gPosition]

	## Game's over if the position is primitive
	if { $primitive != "Undecided" } {
	
	    set gWaitingForHuman true
	    GameOver $gPosition $primitive [peek $gMovesSoFar]
	    
	} else {

	    ## Handle Predictions if they're turned on
	    
	    global gPredictions
	    
	    if { $gPredictions } {
		global gPredString
		GetPredictions
	    .middle.f3.cMRight itemconfigure Predictions \
		    -text [format "Predictions: %s" $gPredString] 
		update idletasks
	    }
	    
	    if { [PlayerIsComputer] } {
		DoComputerMove
		set gWaitingForHuman false
		after 800
		update
	    } else {
		DoHumanMove
		set gWaitingForHuman true
	    }
	}
    }
}

#############################################################################
##
## SwitchWhoseTurn
##
## Switches from the left player's turn to the right player's and vice-versa
## 
## Args: none
##
## Requires: NewGame has been called
##
#############################################################################

proc SwitchWhoseTurn {} {
    global gWhoseTurn

    if { $gWhoseTurn == "Left" } {
	set gWhoseTurn "Right"
    } else {
	set gWhoseTurn "Left"
    }
}
    
#############################################################################
##
## DoComputerMove
##
## This function gets the computer's move from the database and makes it
## 
## Args: none
##
## Requires: The state of the board is such that there are no moves being
##           shown at this time
##
#############################################################################

proc DoComputerMove { } {

    global gPosition gGameSoFar gMovesSoFar

    set theMove [C_GetComputersMove $gPosition]    

    set oldPosition $gPosition

    set gPosition [C_DoMove $gPosition $theMove]

    set gGameSoFar [push $gGameSoFar $gPosition]

    set gMovesSoFar [push $gMovesSoFar $theMove]

    HandleComputersMove .middle.f2.cMain $oldPosition $theMove $gPosition

    if { [expr ![C_GoAgain $oldPosition $theMove]] } {
	SwitchWhoseTurn
    }

}

#############################################################################
##
## HandleComputersMove
##
## This function handles the  move for a computer player
## This should be overwritten for modules which support n-phase moves,
## with n > 1
##
#############################################################################

proc HandleComputersMove { c oldPos theMove Position } {

  GS_HandleMove $c $oldPos $theMove $Position

}


#############################################################################
##
## DoHumanMove
##
## This function shows all the moves and relinquishes control so that the
## user can input the move
## 
## Args: none
##
## Requires: The state of the board is such that there are no moves being
##           shown at this time
##
#############################################################################

proc DoHumanMove { } {

    global gPosition gMoveType

    GS_ShowMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]

}

#############################################################################
##
## ReturnFromHumanMove
##
## This function is called from the Game Specific Functions and gives
## control back to gamesman
## 
## Args: the Move
##
## Requires: The Moves on the Game Board are still being shown
##
#############################################################################

proc ReturnFromHumanMove { theMove } {
    global gamestarted
    if {$gamestarted} {
        ReturnFromHumanMoveHelper $theMove
    }
}

proc ReturnFromHumanMoveHelper { theMove } {
        
    global gPosition gGameSoFar gMovesSoFar gMoveType

    set primitive [C_Primitive $gPosition]

    set PositionValueList [C_GetValueMoves $gPosition]
        
    if { $primitive == "Undecided" &&
         [containskey $theMove $PositionValueList] } {
        
        GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]
        
        set oldPosition $gPosition
                
        set gPosition [C_DoMove $gPosition $theMove]
                
        set gGameSoFar [push $gGameSoFar $gPosition]
        
        set gMovesSoFar [push $gMovesSoFar $theMove]
                
        GS_HandleMove .middle.f2.cMain $oldPosition $theMove $gPosition

	if { [expr ![C_GoAgain $oldPosition $theMove]] } {
	    SwitchWhoseTurn
	}

        DriverLoop

    }
}


#############################################################################
##
## GameOver
##
## This function is called from the driver loop to signify a game is over.
## 
## Args: the position, the Game Value (win, lose or tie)
##
## Requires: The Moves on the Game Board are not shown
##
#############################################################################

proc GameOver { position gameValue lastMove } {

    global gPosition gGameSoFar gWhoseTurn gLeftName gRightName 
    global gLeftPiece gRightPiece

    set previousPos [peek [pop $gGameSoFar]]

    set WhoWon Nobody

    set WhichPieceWon Nobody

    if { $gWhoseTurn == "Right" } {

        if { $gameValue == "Win" } {
            
            set WhoWon $gRightName

            set WhichPieceWon $gRightPiece

        } elseif { $gameValue == "Lose" } {

            set WhoWon $gLeftName

            set WhichPieceWon $gLeftPiece

        }

    } else {
        
        if { $gameValue == "Win" } {
            
            set WhoWon $gLeftName

            set WhichPieceWon $gLeftPiece

        } elseif { $gameValue == "Lose" } {

            set WhoWon $gRightName

            set WhichPieceWon $gRightPiece

        }
    }

    if { $gameValue == "Tie" } {
        set message [concat GAME OVER: It's a TIE!]
        SendMessage $message
    } else {
        set message [concat GAME OVER: $WhoWon Wins!]
	SendMessage $message
    }

    .middle.f3.cMRight itemconfigure Predictions \
	-text [format "%s" $message] 
    update idletasks

    GS_GameOver .middle.f2.cMain $gPosition $gameValue $WhichPieceWon $WhoWon $lastMove

    DisableMoves
}


#############################################################################
##
## DisableMoves/EnableMoves
##
## Disables or Enables the radiobuttons which show All/Value/Best Moves
## 
## Args: None
##
## Requires: Nothing
##
#############################################################################

proc DisableMoves {} {

#   .winPlayOptions.fRadio.butAllMoves configure -state disabled
#   .winPlayOptions.fRadio.butValueMoves configure -state disabled
#   .winPlayOptions.fRadio.butBestMoves configure -state disabled

}

proc EnableMoves {} {

#    .winPlayOptions.fRadio.butAllMoves configure -state normal
#    .winPlayOptions.fRadio.butValueMoves configure -state normal
#    .winPlayOptions.fRadio.butBestMoves configure -state normal

}

#############################################################################
##
## ToggleMoves
##
## Called When Changing Between All/Value/Best Moves
##
## Args:  A string corresponding to all, value, or best
##
## Requires: Moves are currently shown and Game is not over
##
#############################################################################

proc ToggleMoves { moveType } {

    global gMoveType gPosition

    ChangeMoveType $gMoveType $moveType $gPosition [C_GetValueMoves $gPosition]

    set gMoveType $moveType

}

#############################################################################
##
## ChangeMoveType
##
## Called when Changing Between All/Value/Best Moves
## 
## Args: 
##
## Requires: Moves are currently shown
##
#############################################################################

proc ChangeMoveType { fromMoveType toMoveType position moveList } {

    global gWaitingForHuman

    GS_HideMoves .middle.f2.cMain $fromMoveType $position $moveList

    if { $gWaitingForHuman } {
	GS_ShowMoves .middle.f2.cMain $toMoveType $position $moveList
    }

}

#############################################################################
##
## Undo
##
## Calls Game Specific Undo to undo the last move
## 
## Args: none
##
## Requires: Moves are currently shown
##
#############################################################################

proc Undo { } {
    
    UndoHelper
    GetPredictions
    global gPredString
    .middle.f3.cMRight itemconfigure Predictions \
	    -text [format "Predictions: %s" $gPredString] 
    update idletasks
    
    DriverLoop
}

proc UndoHelper { } {
    
    global gPosition gMovesSoFar gGameSoFar gMoveType
    
    if { [llength $gGameSoFar] != 1 } {
        
        set primitive [C_Primitive $gPosition]
        
        if { $primitive == "Undecided" } {
            
            GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]
            
        } else {

            GS_UndoGameOver .middle.f2.cMain $gPosition

        }
        
        set undoOnce [pop $gGameSoFar]
        
        GS_HandleUndo .middle.f2.cMain [peek $gGameSoFar] [peek $gMovesSoFar] [peek $undoOnce]
        
        set gGameSoFar $undoOnce
        
        set gPosition [peek $gGameSoFar]
        
	set undoneMove [peek $gMovesSoFar]
        set gMovesSoFar [pop $gMovesSoFar]

	if { [expr ![C_GoAgain $gPosition $undoneMove]] } {
	    SwitchWhoseTurn
	}
        
        if { [PlayerIsComputer] } {
            
            UndoHelper

	}
        
    }

}

#############################################################################
##
## PlayerIsComputer
##
## Returns true or false if the player whose turn it is currently is controlled
## by a computer
## 
## Args: Nothing
##
## Requires: Nothing
##
#############################################################################

proc PlayerIsComputer { } {

    global gWhoseTurn gRightHumanOrComputer gLeftHumanOrComputer

    if { $gWhoseTurn == "Left" } {
        return [expr { $gLeftHumanOrComputer == "Computer"}]
    } else {
        return [expr { $gRightHumanOrComputer == "Computer"}]
    }
}

proc ReturnToGameSpecificOptions {} {
    GS_Deinitialize .middle.f2.cMain
    global gGameSolved
    set gGameSolved false
}

#############################################################################
##
## SendMessage
##
## Displays whose turn it is or who's won
## 
## Args: string
##
## Requires: Nothing
##
#############################################################################

proc SendMessage { arg } {
    
}

#############################################################################
##
## GetPredictions
##
## Get predictions from the C code
## 
## Args: Nothing
##
## Requires: Nothing
##
#############################################################################

proc GetPredictions {} {

    global gPosition gPredString gWhoseTurn
    global gLeftName gRightName

    if { [C_Primitive $gPosition] != "Undecided" } {
	set gPredString ""
    } else {

	### Get the value, the player and set the prediction
	set theValue      [C_GetValueOfPosition $gPosition]
	set theRemoteness [C_Remoteness $gPosition]
	
	if { $gWhoseTurn == "Left" } {
	    set playersName $gLeftName
	} else {
	    set playersName $gRightName
	}
	
	if { $theValue == "Tie" && $theRemoteness == 255 } {
	    set prediction [format "%s should Draw" $playersName]
	} else {
	    set prediction [format "%s should %s in %s" $playersName $theValue $theRemoteness]
	}

	### And place it in the field if the button is on.
	set gPredString $prediction
    }
}

proc GetAllGameOptions {} {
    global gRule0
    set settings [GetGameSpecificOptions]
    set settings [linsert $settings 0 [expr !$gRule0]]
    return $settings
}

proc GetGameSpecificOptions {} {
    global gNumberOfRules
    set settings ""

    for { set i 1 } { $i < $gNumberOfRules } { incr i } {
	global gRule$i
	set settings [concat $settings [subst $[subst gRule$i]]]
    }
    return $settings
}

# argv etc
proc main {kRootDir} {
    source "$kRootDir/../tcl/InitWindow.tcl"
    InitConstants
    C_Initialize
    C_InitializeDatabases
    GS_InitGameSpecific
    InitWindow $kRootDir
}
