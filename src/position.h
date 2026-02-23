//////////////////////////////////////////////////////////////////////
//
//  FILE:       position.h
//              Position class
//
//  Part of:    Scid (Shane's Chess Information Database)
//  Version:    3.5
//
//  Notice:     Copyright (c) 1999-2003 Shane Hudson.  All rights reserved.
//
//  Author:     Shane Hudson (sgh@users.sourceforge.net)
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "common.h"
#include "movelist.h"
#include <stdio.h>
#include <string>
#include <string_view>

//////////////////////////////////////////////////////////////////////
//  Position:  Constants

const byte  WQ_CASTLE = 1,    WK_CASTLE = 2,
            BQ_CASTLE = 4,    BK_CASTLE = 8;

// SANFlag: since checking if a move is check (to add the "+" to its
//      SAN string) takes time, and checking for mate takes even
//      longer, we specify whether we want this done with a flag.
typedef byte      sanFlagT;
const sanFlagT    SAN_NO_CHECKTEST   = 0,
                  SAN_CHECKTEST      = 1,
                  SAN_MATETEST       = 2;



// Flags that Position::GenerateMoves() recognises:
//
typedef uint genMovesT;
const genMovesT
    GEN_CAPTURES = 1,
    GEN_NON_CAPS = 2,
    GEN_ALL_MOVES = (GEN_CAPTURES | GEN_NON_CAPS);


///////////////////////////////////////////////////////////////////////////
//  Position:  Class definition

class Position
{

private:
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //  Position:  Data structures

    pieceT          Board[66];      // the actual board + a color square
                                    // and a NULL square.
    uint            Count[2];       // count of pieces & pawns each
    byte            Material[16];   // count of each type of piece
    byte            ListPos[64];    // ListPos stores the position in
                                        // List[][] for the piece on
                                        // square x.
    squareT List[2][16];  // list of piece squares for each side

    directionT      Pinned[16];     // For each List[ToMove][x], stores
                                        // whether piece is pinned to its
                                        // own king and dir from king.

    squareT         EPTarget;       // square pawns can EP capture to
    colorT          ToMove;
    ushort          HalfMoveClock;  // Count of halfmoves since last capture
                                    // or pawn move.
    ushort          PlyCounter;
    byte            Castling;       // castling flags
    byte            variant_;       // 0 -> normal; 1 -> chess960
    squareT         castleRookSq_[4];  // start rook squares

    uint            Hash;           // Hash value.
    uint            PawnHash;       // Pawn structure hash value.

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //  Position:  Private Functions

    inline void AddHash (pieceT p, squareT sq);
    inline void UnHash (pieceT p, squareT sq);

    inline void AddToBoard (pieceT p, squareT sq);
    inline void RemoveFromBoard (pieceT p, squareT sq);

    static constexpr unsigned castlingIdx(colorT color, castleDirT side) {
        return 2 * color + side;
    }
    squareT find_castle_rook(colorT col, squareT rsq) const;
    squareT castleRookSq(colorT color, bool king_side) const {
        return castleRookSq_[2 * color + (king_side ? 1 : 0)];
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //  Position:  Public Functions
public:
    Position();
    static const Position& getStdStart();

    void        Clear();        // No pieces on board
    void StdStart() { *this = getStdStart(); }
    errorT      AddPiece (pieceT p, squareT sq);

    bool isChess960() const { return variant_ == 1; }

    // Set and Get attributes -- one-liners
    void SetEPTarget(squareT s) { EPTarget = s; }
    void        SetToMove (colorT c)     { ToMove = c; }
    colorT      GetToMove () const       { return ToMove; }
    bool        WhiteToMove () const     { return ToMove == WHITE; }
    void        SetPlyCounter (ushort x) { PlyCounter = x; }
    ushort GetPlyCounter() const { return PlyCounter; }

    const pieceT* GetBoard() const {
        const_cast<Position*>(this)->Board[COLOR_SQUARE] = COLOR_CHAR[ToMove];
        return Board;
    }

    pieceT GetPiece(squareT sq) const {
        ASSERT(sq < 64);
        return Board[sq];
    }

    // Other one-line methods
    squareT     GetKingSquare (colorT c) const { return List[c][0]; }
    squareT GetKingSquare() const { return List[ToMove][0]; }

    // Castling flags
    bool GetCastling(colorT c, castleDirT dir) const {
        return Castling & (1u << castlingIdx(c, dir));
    }
    bool validCastlingFlag(colorT color, bool king_side) const;

    // Move execution

    void        makeMove(squareT from, squareT to, pieceT promo, simpleMoveT& res) const;
    void        fillMove(simpleMoveT& sm) const;
    void        DoSimpleMove(simpleMoveT const& sm);
    void        DoSimpleMove (simpleMoveT * sm);    // move execution ...
    void        UndoSimpleMove(simpleMoveT const& sm);  // ... and taking back

    // Board I/O
    void PrintFEN(char* str) const;

    void setCastling(colorT col, squareT rsq);

   private:
    void ClearCastling(colorT col, castleDirT dir) {
      Castling &= ~(1u << castlingIdx(col, dir));
    }
        void ClearCastlingFlags(colorT c) {
		Castling &= (c == WHITE) ? 0b11111100 : 0b11110011;
	}
};

//////////////////////////////////////////////////////////////////////
//  EOF: position.h
//////////////////////////////////////////////////////////////////////

