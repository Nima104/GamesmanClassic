/************************************************************************
**
** NAME:	db.c
**
** DESCRIPTION:	Generic Database Functions and Database Class Accessors
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

/*
** Needs to be built up to implement The new DB Class abstraction as is
** Found in the Expeimental directory. However we first need to make
** The existing functions abstract.
*/


#include "gamesman.h"
#include "db.h"
#include "memdb.h"
#include "twobitdb.h"
#include "colldb.h"
#include "globals.h"

/* Provide optional support for randomized-hash based collision database, dependent on GMP */
#ifdef HAVE_GMP
#include "univdb.h"
#endif


/* internal function prototypes */
/* default functions common to all db's*/

/*will make this return the function table later*/
void		db_create();
void            db_destroy();
void            db_initialize();

/* these are generic functions that will be executed when the database is uninitialized */
void		db_free			();
VALUE		db_get_value		(POSITION pos);
VALUE		db_put_value		(POSITION pos, VALUE data);
REMOTENESS	db_get_remoteness	(POSITION pos);
void		db_put_remoteness	(POSITION pos, REMOTENESS data);
BOOLEAN		db_check_visited	(POSITION pos);
void		db_mark_visited		(POSITION pos);
void		db_unmark_visited	(POSITION pos);
MEX	       	db_get_mex		(POSITION pos);
void		db_put_mex		(POSITION pos, MEX theMex);
BOOLEAN		db_save_database	();
BOOLEAN		db_load_database	();

/*internal variables*/

DB_Table *db_functions;

/*
** function code
*/
void db_create() {
    
    /*if there is an old database table, get rid of it*/
    db_destroy();

    /* get a new table */
    db_functions = (DB_Table *) SafeMalloc(sizeof(DB_Table));

    /*set all function pointers to NULL, and each database can choose*/
    /*whatever ones they wanna implement and associate them*/
    
    db_functions->get_value = db_get_value;
    db_functions->put_value = db_put_value;
    db_functions->get_remoteness = db_get_remoteness;
    db_functions->put_remoteness = db_put_remoteness;
    db_functions->check_visited = db_check_visited;
    db_functions->mark_visited = db_mark_visited;
    db_functions->unmark_visited = db_unmark_visited;
    db_functions->get_mex = db_get_mex;
    db_functions->put_mex = db_put_mex;
    db_functions->save_database = db_save_database;
    db_functions->load_database = db_load_database;
    db_functions->free_db = db_free;
}

void db_destroy() {
    if(db_functions) {
	if(db_functions->free_db)
	    db_functions->free_db();
	SafeFree(db_functions);
    }
}

void db_initialize(){

    if (gTwoBits) {
        db_functions = twobitdb_init();

    } else if(gCollDB){
	db_functions = colldb_init();
    }

#ifdef HAVE_GMP
    else if(gUnivDB) {
	db_functions = univdb_init();
    }
#endif

    else {
	memdb_init(db_functions);
    }

}


void db_free(){
    return ;
}

VALUE db_get_value(POSITION pos){
    printf("DB: Cannot read value of position " POSITION_FORMAT ". The database is uninitialized.\n", pos);
    ExitStageRight();
    exit(0);
}

VALUE db_put_value(POSITION pos, VALUE data){
    printf("DB: Cannot store value of position " POSITION_FORMAT ". The database is uninitialized.\n", pos);
    ExitStageRight();
    exit(0);
}

REMOTENESS db_get_remoteness(POSITION pos){
    return kBadRemoteness;
}

void db_put_remoteness(POSITION pos, REMOTENESS data){
    return;
}

BOOLEAN db_check_visited(POSITION pos){
    return FALSE;
}

void db_mark_visited(POSITION pos){
    return;
}

void db_unmark_visited(POSITION pos){
    return;
}

MEX db_get_mex(POSITION pos){
    return kBadMexValue;
}

void db_put_mex(POSITION pos, MEX theMex){
    return;
}

BOOLEAN db_save_database(){
    printf("NOTE: The database cannot be saved.");
    return FALSE;
}

BOOLEAN db_load_database(){
    printf("NOTE: The database cannot be loaded.");
    return FALSE;
}

void CreateDatabases()
{
    db_create();
}

void InitializeDatabases()
{
    db_initialize();
}

void DestroyDatabases()
{
    db_destroy();
}

VALUE StoreValueOfPosition(POSITION position, VALUE value)
{
    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);
    return db_functions->put_value(position,value);
}


VALUE GetValueOfPosition(POSITION position)
{
    if(((kLoopy && gMenuMode != Analysis) || gMenuMode == Evaluated) && gSymmetries)
	position = gCanonicalPosition(position);
    return db_functions->get_value(position);
}


REMOTENESS Remoteness(POSITION position)
{
    if(((kLoopy && gMenuMode != Analysis) || gMenuMode == Evaluated) && gSymmetries)
	position = gCanonicalPosition(position);
    return db_functions->get_remoteness(position);
}
    

void SetRemoteness (POSITION position, REMOTENESS remoteness)
{
    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);
    db_functions->put_remoteness(position,remoteness);
}
 

BOOLEAN Visited(POSITION position)
{
    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);
    return db_functions->check_visited(position);
}


void MarkAsVisited (POSITION position)
{
    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);
    db_functions->mark_visited(position);
}

void UnMarkAsVisited (POSITION position)
{
    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);
    db_functions->unmark_visited(position);
}

void UnMarkAllAsVisited()
{
    int i;

    for(i = 0; i < gNumberOfPositions; i++)
    {
        db_functions->unmark_visited(i);
    }

}


void MexStore(POSITION position, MEX theMex)
{
    /* do we need this?? */
    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);

    db_functions->put_mex(position, theMex);
}

MEX MexLoad(POSITION position)
{
    /* do we need this?? */
    if(kLoopy && gSymmetries)
	position = gCanonicalPosition(position);

    return db_functions->get_mex(position);
}

BOOLEAN SaveDatabase() {
    return db_functions->save_database();    
}

BOOLEAN LoadDatabase() {
    return db_functions->load_database();
}
