//////////////////////////////////////////////////////////////////////
//
//  FILE:       movelist.h
//              MoveList class
//
//  Part of:    Scid (Shane's Chess Information Database)
//  Version:    3.4
//
//  Notice:     Copyright (c) 1999-2002 Shane Hudson.  All rights reserved.
//              Copyright (c) 2016 Fulvio Benini.  All rights reserved.
//
//  Author:     Shane Hudson (sgh@users.sourceforge.net)
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "common.h"

//////////////////////////////////////////////////////////////////////
//  MoveList:  Constants

const uint  MAX_LEGAL_MOVES = 256;  // max. length of the moves list


///////////////////////////////////////////////////////////////////////////
//  MoveList:  Data Structures

// *** SimpleMove: less expensive to store than a full move as defined
//      in game.h, but still fully undoable.
//
struct simpleMoveT
{
    squareT  from;
    squareT  to;
    pieceT   promote; // EMPTY if not a promotion, type (no color) otherwise
    pieceT   movingPiece : 7;
    pieceT   castling : 1;
    byte     pieceNum;
    byte     capturedNum;
    pieceT   capturedPiece;
    squareT  capturedSquare; // ONLY different to "to" field if this capture
                            //    is an en passant capture.
    byte     castleFlags;    // pre-move information
    squareT  epSquare;       // pre-move information
    ushort   oldHalfMoveClock;

	bool isNullMove() const {
		return from == to && from != NULL_SQUARE &&
		       piece_Type(movingPiece) == KING;
	}

	/// Returns:
	///  +2 for king side castle
	///  -2 for queen side castle
	///  0 (false) if it is not a castle moves.
	int isCastle() const {
		if (castling)
			return to > from ? 2 : -2;

		return 0;
	}

	static simpleMoveT const empty() {
		simpleMoveT sm;
		sm.from = NULL_SQUARE;
		sm.to = NULL_SQUARE;
		return sm;
	}

	bool isEmpty() const { return from == NULL_SQUARE && to == NULL_SQUARE; }

	/// Converts the move to long algebraic notation.
	/// @return a pointer one past the last char written.
	template <typename OutputIt> OutputIt toLongNotation(OutputIt dest) const {
		if (from == to) {
			// UCI standard for null move
			*dest++ = '0';
			*dest++ = '0';
			*dest++ = '0';
			*dest++ = '0';
		} else {
			*dest++ = square_FyleChar(from);
			*dest++ = square_RankChar(from);
			*dest++ = square_FyleChar(to);
			*dest++ = square_RankChar(to);
			if (promote != EMPTY) {
				constexpr const char promoChars[] = "  qrbn ";
				*dest++ = promoChars[piece_Type(promote)];
			}
		}
		return dest;
	}
};
