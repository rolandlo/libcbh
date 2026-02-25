//////////////////////////////////////////////////////////////////////
//
//  FILE:       common.h
//              Common macros, structures and constants.
//
//  Part of:    Scid (Shane's Chess Information Database)
//  Version:    3.6.6
//
//  Notice:     Copyright (c) 2000-2004  Shane Hudson.  All rights reserved.
//
//  Author:     Shane Hudson (sgh@users.sourceforge.net)
//              Copyright (c) 2006-2007 Pascal Georges
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "board_def.h"
#include "error.h"
#include <assert.h>
#include <cstddef>
#include <stdint.h>
#define ASSERT(f) assert(f)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TYPE DEFINITIONS

//  General types
typedef unsigned char byte;      //  8 bit unsigned
typedef uint16_t ushort;         // 16 bit unsigned
typedef uint32_t uint;           // 32 bit unsigned
typedef int32_t  sint;           // 32 bit signed

//  Chess Types
typedef byte directionT;                     // e.g. UP_LEFT
typedef byte                    castleDirT;  // LEFT or RIGHT

typedef char    sanStringT [ 10];   // SAN Move Notation

enum fileModeT {
    FMODE_None = 0,
    FMODE_ReadOnly,
    FMODE_WriteOnly,
    FMODE_Both,
    FMODE_Create
};


//  Game Information types

typedef uint            gamenumT;
typedef ushort          eloT;
typedef ushort          ecoT;
typedef char            ecoStringT [6];   /* "A00j1" */

const ecoT ECO_None = 0;

// Rating types:

const byte RATING_Elo = 0;
const byte RATING_Rating = 1;
const byte RATING_Rapid = 2;
const byte RATING_ICCF = 3;
const byte RATING_USCF = 4;
const byte RATING_DWZ = 5;
const byte RATING_BCF = 6;

// Result Type

const uint NUM_RESULT_TYPES = 4;
typedef byte    resultT;
const resultT
    RESULT_None  = 0,
    RESULT_White = 1,
    RESULT_Black = 2,
    RESULT_Draw  = 3;

const uint RESULT_SCORE[4] = { 1, 2, 0, 1 };
const char RESULT_CHAR [4]       = { '*',  '1',    '0',    '='       };
const char RESULT_STR [4][4]     = { "*",  "1-0",  "0-1",  "=-="     };
const char RESULT_LONGSTR [4][8] = { "*",  "1-0",  "0-1",  "1/2-1/2" };
const resultT RESULT_OPPOSITE [4] = {
    RESULT_None, RESULT_Black, RESULT_White, RESULT_Draw
};


const castleDirT  QSIDE = 0,  KSIDE = 1;


// Minor piece definitions, used for searching by material only:
const pieceT  WM = 16, BM = 17;

const uint MAX_PIECE_TYPES = 18;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SQUARES AND SQUARE MACROS

inline char square_FyleChar(squareT sq) { return square_Fyle(sq) + 'a'; }

inline char square_RankChar(squareT sq) { return square_Rank(sq) + '1'; }

// Directions:

const directionT NULL_DIR = 0, UP = 1, DOWN = 2, LEFT = 4, RIGHT = 8;

//////////////////////////////////////////////////////////////////////
//  EOF:  common.h
//////////////////////////////////////////////////////////////////////

