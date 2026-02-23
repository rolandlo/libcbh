//////////////////////////////////////////////////////////////////////
//
//  FILE:       position.cpp
//              Position class methods
//
//  Part of:    Scid (Shane's Chess Information Database)
//  Version:    3.5
//
//  Notice:     Copyright (c) 1999-2003 Shane Hudson.  All rights reserved.
//
//  Author:     Shane Hudson (sgh@users.sourceforge.net)
//
//////////////////////////////////////////////////////////////////////

#include "position.h"

#include <algorithm>
#include <array>
#include <vector>

#include "common.h"
#include "hash.h"
#include "sqmove.h"

namespace {
bool valid_sqlist(pieceT* begin, size_t n, pieceT* board) {
	auto unique_sq = std::vector<pieceT>(begin, begin + n);
	std::sort(unique_sq.begin(), unique_sq.end());
	unique_sq.erase(std::unique(unique_sq.begin(), unique_sq.end()),
	                unique_sq.end());
	auto unique_col = std::vector<pieceT>(n);
	std::transform(begin, begin + n, unique_col.begin(),
	               [&](auto sq) { return piece_Color(board[sq]); });
	std::sort(unique_col.begin(), unique_col.end());
	unique_col.erase(std::unique(unique_col.begin(), unique_col.end()),
	                 unique_col.end());
	auto kings = std::vector<pieceT>(n);
	std::transform(begin, begin + n, kings.begin(),
	               [&](auto sq) { return piece_Type(board[sq]); });
	return unique_sq.size() == n     // no duplicates
	       && unique_col.size() == 1 // not EMPTY and all of same side
	       && kings[0] == KING       // only 1 king with idx == 0;
	       && std::count(kings.begin(), kings.end(), KING) == 1;
}
} // namespace

inline void
Position::AddHash (pieceT p, squareT sq)
{
    HASH (Hash, p, sq);
    if (piece_Type(p) == PAWN) {
        HASH (PawnHash, p, sq);
    }
}

inline void
Position::UnHash (pieceT p, squareT sq)
{
    UNHASH (Hash, p, sq);
    if (piece_Type(p) == PAWN) {
        UNHASH (PawnHash, p, sq);
    }
}

inline void
Position::AddToBoard (pieceT p, squareT sq)
{
    ASSERT (Board[sq] == EMPTY);
    Board[sq] = p;
    AddHash (p, sq);
}

inline void
Position::RemoveFromBoard (pieceT p, squareT sq)
{
    ASSERT (Board[sq] == p);
    Board[sq] = EMPTY;
    UnHash (p, sq);
}

// Sanity checks for castling flags.
// Return true if the corresponding castling flag is set, there is a rook in the
// expected square, and the king is in the same first rank of the rook.
bool Position::validCastlingFlag(colorT color, bool king_side) const {
	if (!GetCastling(color, king_side ? KSIDE : QSIDE))
		return false;

	const auto kingFrom = GetKingSquare(color);
	const auto rookFrom = castleRookSq(color, king_side);
	return Board[rookFrom] == piece_Make(color, ROOK) &&
	       square_Rank(kingFrom) == square_Rank(rookFrom) &&
	       square_Rank(kingFrom) == rank_Relative(color, RANK_1);
}

//////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS

Position::Position() {
    // Setting up a valid board is left to StdStart() or Clear().
    Board [COLOR_SQUARE] = EMPTY;
    Board [NULL_SQUARE] = END_OF_BOARD;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Position::Clear():
//      Clear the board and associated structures.
//
void
Position::Clear (void)
{
    int i;
    for (i=A1; i <= H8; i++) { Board[i] = EMPTY; }
    for (i=WK; i <= BP; i++) {
      Material[i] = 0;
    }
    Count[WHITE] = Count[BLACK] = 0;
    EPTarget = NULL_SQUARE;
    Castling = 0;
    variant_ = 0;
    std::fill_n(castleRookSq_, 4, 0);
    Board [NULL_SQUARE] = END_OF_BOARD;
    PlyCounter = 0;
    HalfMoveClock = 0;
    Hash = 0;
    PawnHash = 0;
    return;
}

squareT Position::find_castle_rook(colorT col, squareT rsq) const {
	const auto ksq = GetKingSquare(col);
	if (square_Rank(ksq) == square_Rank(rsq)) {
		const auto rook = piece_Make(col, ROOK);
		while (Board[rsq] != rook && rsq != ksq) {
			if (rsq < ksq)
				++rsq;
			else
				--rsq;
		}
	}
	return rsq;
}

void Position::setCastling(colorT col, squareT rsq) {
	static_assert(1 << castlingIdx(WHITE, QSIDE) == 1);
	static_assert(1 << castlingIdx(WHITE, KSIDE) == 2);
	static_assert(1 << castlingIdx(BLACK, QSIDE) == 4);
	static_assert(1 << castlingIdx(BLACK, KSIDE) == 8);

	const auto ksq = GetKingSquare(col);
	rsq = find_castle_rook(col, rsq);
	const auto dir = square_Fyle(ksq) < square_Fyle(rsq) ? KSIDE : QSIDE;
	const auto idx = castlingIdx(col, dir);
	castleRookSq_[idx] = rsq;
	Castling |= 1u << idx;
	const auto std_rsq = (dir == QSIDE) ? square_Relative(col, A1)
	                                    : square_Relative(col, H1);
	if (ksq != square_Relative(col, E1) || rsq != std_rsq)
		variant_ = 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Position::StdStart():
//      Set up the standard chess starting position. For performance the data is copied from a 
//      template.
//
const Position& Position::getStdStart()
{
    static Position startPositionTemplate;
    static bool init = true;

    if (init){
        init = false;
        Position* p = &startPositionTemplate;
        p->Clear();
        p->Material[WK] = p->Material[BK] = 1;
        p->Material[WQ] = p->Material[BQ] = 1;
        p->Material[WR] = p->Material[BR] = 2;
        p->Material[WB] = p->Material[BB] = 2;
        p->Material[WN] = p->Material[BN] = 2;
        p->Material[WP] = p->Material[BP] = 8;
        p->Count[WHITE] = p->Count[BLACK] = 16;

        p->AddToBoard(WK, E1);  p->List[WHITE][0] = E1;  p->ListPos[E1] = 0;
        p->AddToBoard(BK, E8);  p->List[BLACK][0] = E8;  p->ListPos[E8] = 0;
        p->AddToBoard(WR, A1);  p->List[WHITE][1] = A1;  p->ListPos[A1] = 1;
        p->AddToBoard(BR, A8);  p->List[BLACK][1] = A8;  p->ListPos[A8] = 1;
        p->AddToBoard(WN, B1);  p->List[WHITE][2] = B1;  p->ListPos[B1] = 2;
        p->AddToBoard(BN, B8);  p->List[BLACK][2] = B8;  p->ListPos[B8] = 2;
        p->AddToBoard(WB, C1);  p->List[WHITE][3] = C1;  p->ListPos[C1] = 3;
        p->AddToBoard(BB, C8);  p->List[BLACK][3] = C8;  p->ListPos[C8] = 3;
        p->AddToBoard(WQ, D1);  p->List[WHITE][4] = D1;  p->ListPos[D1] = 4;
        p->AddToBoard(BQ, D8);  p->List[BLACK][4] = D8;  p->ListPos[D8] = 4;
        p->AddToBoard(WB, F1);  p->List[WHITE][5] = F1;  p->ListPos[F1] = 5;
        p->AddToBoard(BB, F8);  p->List[BLACK][5] = F8;  p->ListPos[F8] = 5;
        p->AddToBoard(WN, G1);  p->List[WHITE][6] = G1;  p->ListPos[G1] = 6;
        p->AddToBoard(BN, G8);  p->List[BLACK][6] = G8;  p->ListPos[G8] = 6;
        p->AddToBoard(WR, H1);  p->List[WHITE][7] = H1;  p->ListPos[H1] = 7;
        p->AddToBoard(BR, H8);  p->List[BLACK][7] = H8;  p->ListPos[H8] = 7;

        for (uint i=0; i < 8; i++) {
            p->AddToBoard(WP, A2+i); p->List[WHITE][i+8] = A2+i; p->ListPos[A2+i] = i+8;
            p->AddToBoard(BP, A7+i); p->List[BLACK][i+8] = A7+i; p->ListPos[A7+i] = i+8;
        }

        p->setCastling(WHITE, A1);
        p->setCastling(WHITE, H1);
        p->setCastling(BLACK, A8);
        p->setCastling(BLACK, H8);
        p->EPTarget = NULL_SQUARE;
        p->ToMove = WHITE;
        p->PlyCounter = 0;
        p->HalfMoveClock = 0;
        p->Board [NULL_SQUARE] = END_OF_BOARD;
        p->Hash = stdStartHash;
        p->PawnHash = stdStartPawnHash;
    }
    return startPositionTemplate;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Position::AddPiece():
//      Add a piece to the board and piecelist.
//      Checks that a side cannot have more than 16 pieces or more
//      than one king.
//
errorT
Position::AddPiece (pieceT p, squareT sq)
{
    ASSERT (p != EMPTY);
    colorT c = piece_Color(p);
    if ((c != WHITE && c != BLACK) || Count[c] > 15)
        return ERROR_PieceCount;

    if (piece_Type(p) == KING) {
        // Check there is not already a King:
        if (Material[p] > 0) { return ERROR_PieceCount; }

        // King is always at the start of the piecelist, so move the piece
        // already at location 0 if there is one:
        if (Count[c] > 0) {
            squareT oldsq = List[c][0];
            List[c][Count[c]] = oldsq;
            ListPos[oldsq] = Count[c];
        }
        List[c][0] = sq;
        ListPos[sq] = 0;
    } else {
        ListPos[sq] = Count[c];
        List[c][Count[c]] = sq;
    }
    Count[c]++;
    Material[p]++;
    AddToBoard (p, sq);
    return OK;
}

/// Make a simpleMoveT.
/// If @e promo != INVALID_PIECE it is a special move:
/// promo == PAWN -> null move
/// from != to -> promotion
/// from == to && PAWN == KING -> castle kingside
/// from == to && PAWN != KING -> castle queenside
void Position::makeMove(squareT from, squareT to, pieceT promo,
                        simpleMoveT& res) const {
	res.castling = 0;
	if (promo == EMPTY || promo == INVALID_PIECE) { // NORMAL MOVE
		res.from = from;
		res.to = to;
		res.promote = EMPTY;
	} else if (promo == PAWN) { // NULL MOVE
		res.from = GetKingSquare(ToMove);
		res.to = res.from;
		res.promote = EMPTY;
		res.movingPiece = KING;
	} else if (from != to) { // PROMOTION
		res.from = from;
		res.to = to;
		res.promote = promo;
	} else { // CASTLING
		res.from = GetKingSquare(ToMove);
		if (isChess960()) {
			res.to = castleRookSq(ToMove, promo == KING);
		} else {
			res.to = res.from;
			res.to += promo == KING ? 2 : -2;
		}
		res.promote = EMPTY;
		res.castling = 1;
	}
	fillMove(res);
}

/// Use the current position to retrieve all the information needed to create a
/// SimpleMove which can also be used in UndoSimpleMove.
void Position::fillMove(simpleMoveT& sm) const {
	const auto from = sm.from;
	const auto to = sm.to;
	sm.movingPiece = GetPiece(sm.from);
	sm.pieceNum = ListPos[from];
	sm.castleFlags = Castling;
	sm.epSquare = EPTarget;
	sm.oldHalfMoveClock = HalfMoveClock;
	sm.capturedSquare = to;
	if (sm.isNullMove() || sm.isCastle()) {
		sm.capturedPiece = EMPTY;
	} else {
		sm.capturedPiece = GetPiece(to);
	}

	// Handle en passant capture:
	if (piece_Type(sm.movingPiece) == PAWN && sm.capturedPiece == EMPTY &&
	    square_Fyle(from) != square_Fyle(to)) {
		// This was an EP capture. We do not need to check it was a capture
		// since if a pawn lands on EPTarget it must capture to get there.
		sm.capturedSquare = WhiteToMove() ? to - 8 : to + 8;
		sm.capturedPiece = GetPiece(sm.capturedSquare);
		ASSERT(sm.capturedPiece == piece_Make(color_Flip(GetToMove()), PAWN));
	}

	if (sm.capturedPiece != EMPTY) {
		sm.capturedNum = ListPos[sm.capturedSquare];
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Position::DoSimpleMove():
//      Make the move on the board and update all the necessary
//      fields in the simpleMove structure so it can be undone.
//
void
Position::DoSimpleMove (simpleMoveT * sm)
{
    ASSERT (sm != NULL);
    // update move fields that (maybe) have not yet been set:
	fillMove(*sm);
	DoSimpleMove(*sm);
}

void Position::DoSimpleMove(simpleMoveT const& sm) {
    const auto from = sm.from;
    const auto to = sm.to;
    const auto promo = sm.promote;
    const auto movingPiece = GetPiece(from);
    const auto ptype = piece_Type(movingPiece);
    const auto pieceNum = ListPos[from];
    const auto color = ToMove;
    const auto enemy = color_Flip(color);

    HalfMoveClock++;
    PlyCounter++;
    EPTarget = NULL_SQUARE;
    ToMove = enemy;

	auto addPiece = [&](auto idx, auto pieceType, squareT destSq) {
		List[color][idx] = destSq;
		ListPos[destSq] = idx;
		AddToBoard(piece_Make(color, pieceType), destSq);
	};

	if (sm.isNullMove())
		return;

	if (ptype == KING) {
		ClearCastlingFlags(color);
		if (auto castleSide = sm.isCastle()) {
			squareT rookfrom, rookto, kingTo;
			if (castleSide > 0) {
				kingTo = square_Relative(color, G1);
				rookfrom = castleRookSq(color, true);
				rookto = kingTo - 1;
			} else {
				kingTo = square_Relative(color, C1);
				rookfrom = castleRookSq(color, false);
				rookto = kingTo + 1;
			}
			const int kingIdx = 0;
			const int rookIdx = ListPos[rookfrom];
			RemoveFromBoard(piece_Make(color, ROOK), rookfrom);
			RemoveFromBoard(piece_Make(color, KING), GetKingSquare(color));
			addPiece(kingIdx, KING, kingTo);
			addPiece(rookIdx, ROOK, rookto);

			ASSERT(valid_sqlist(List[WHITE], Count[WHITE], Board));
			ASSERT(valid_sqlist(List[BLACK], Count[BLACK], Board));
			return;
		}
	}

    // handle captures:
    if (sm.capturedPiece != EMPTY) {
        ASSERT(piece_Type(sm.capturedPiece) != KING);
        ASSERT(piece_Color(sm.capturedPiece) == enemy);
        // update opponents List of pieces
        Count[enemy]--;
        ListPos[List[enemy][Count[enemy]]] = sm.capturedNum;
        List[enemy][sm.capturedNum] = List[enemy][Count[enemy]];
        Material[sm.capturedPiece]--;
        HalfMoveClock = 0;
        RemoveFromBoard (sm.capturedPiece, sm.capturedSquare);
    }

    // now make the move:
    RemoveFromBoard(movingPiece, from);
    if (promo != EMPTY) {
        ASSERT(movingPiece == piece_Make(color, PAWN));
        Material[movingPiece]--;
        Material[piece_Make(color, promo)]++;
        addPiece(pieceNum, promo, to);
    } else {
        addPiece(pieceNum, ptype, to);
    }

    // Handle clearing of castling flags:
    if (Castling) {
        // See if a rook moved or was captured:
		if (from == castleRookSq(color, false))
			ClearCastling(color, QSIDE);
		if (from == castleRookSq(color, true))
			ClearCastling(color, KSIDE);
		if (to == castleRookSq(enemy, false))
			ClearCastling(enemy, QSIDE);
		if (to == castleRookSq(enemy, true))
			ClearCastling(enemy, KSIDE);
    }

    // Set the EPTarget square, if a pawn advanced two squares and an
    // enemy pawn is on a square where en passant may be possible.
    if (ptype == PAWN) {
        rankT fromRank = square_Rank(from);
        rankT toRank = square_Rank(to);
        if (fromRank == RANK_2  &&  toRank == RANK_4
              &&  (Board[square_Move(to,LEFT)] == BP
                     ||  Board[square_Move(to,RIGHT)] == BP)) {
            EPTarget = square_Move(from, UP);
        }
        if (fromRank == RANK_7  &&  toRank == RANK_5
              &&  (Board[square_Move(to,LEFT)] == WP
                     ||  Board[square_Move(to,RIGHT)] == WP)) {
            EPTarget = square_Move(from, DOWN);
        }
        HalfMoveClock = 0; // 50-move clock resets on pawn moves.
	}

	ASSERT(valid_sqlist(List[WHITE], Count[WHITE], Board));
	ASSERT(valid_sqlist(List[BLACK], Count[BLACK], Board));
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Position::UndoSimpleMove():
//      Take back a simple move that has been made with DoSimpleMove().
//
void Position::UndoSimpleMove(simpleMoveT const& sm) {
    const squareT from = sm.from;
    const squareT to = sm.to;
    const auto pieceNum = ListPos[to];
    pieceT p = Board[to];
    EPTarget = sm.epSquare;
    Castling = sm.castleFlags;
    HalfMoveClock = sm.oldHalfMoveClock;
    PlyCounter--;
    ToMove = color_Flip(ToMove);

    // Check for a null move:
    if (sm.isNullMove()) {
        return;
    }

	auto addPiece = [&](auto idx, auto pieceType, squareT destSq) {
		List[ToMove][idx] = destSq;
		ListPos[destSq] = idx;
		AddToBoard(piece_Make(ToMove, pieceType), destSq);
	};

	// handle Castling:
		if (auto castleSide = sm.isCastle()) {
			const auto kingSq = GetKingSquare(ToMove);
			squareT rookfrom, rookto;
			if (castleSide > 0) {
				rookfrom = kingSq - 1;
				rookto = castleRookSq(ToMove, true);
			} else {
				rookfrom = kingSq + 1;
				rookto = castleRookSq(ToMove, false);
			}
			const int kingIdx = 0;
			const int rookIdx = ListPos[rookfrom];
			RemoveFromBoard(piece_Make(ToMove, KING), kingSq);
			RemoveFromBoard(piece_Make(ToMove, ROOK), rookfrom);
			addPiece(rookIdx, ROOK, rookto);
			addPiece(kingIdx, KING, from);

		    ASSERT(valid_sqlist(List[WHITE], Count[WHITE], Board));
		    ASSERT(valid_sqlist(List[BLACK], Count[BLACK], Board));
		    return;
		}

    // Handle a capture: insert piece back into piecelist.
    // This works for EP captures too, since the square of the captured
    // piece is in the "capturedSquare" field rather than assuming the
    // value of the "to" field. The only time these two fields are
    // different is for an en passant move.
    if (sm.capturedPiece != EMPTY) {
        colorT c = color_Flip(ToMove);
        ListPos[List[c][sm.capturedNum]] = Count[c];
        ListPos[sm.capturedSquare] = sm.capturedNum;
        List[c][Count[c]] = List[c][sm.capturedNum];
        List[c][sm.capturedNum] = sm.capturedSquare;
        Material[sm.capturedPiece]++;
        Count[c]++;
    }

    // handle promotion:
    if (sm.promote != EMPTY) {
        Material[p]--;
        RemoveFromBoard (p, to);
        p = piece_Make(ToMove, PAWN);
        Material[p]++;
        AddToBoard (p, to);
    }

    // now make the move:
    List[ToMove][pieceNum] = from;
    ListPos[from] = pieceNum;
    RemoveFromBoard (p, to);
    AddToBoard (p, from);
    if (sm.capturedPiece != EMPTY) {
        AddToBoard (sm.capturedPiece, sm.capturedSquare);
    }

    ASSERT(valid_sqlist(List[WHITE], Count[WHITE], Board));
    ASSERT(valid_sqlist(List[BLACK], Count[BLACK], Board));
}

void Position::PrintFEN(char* str) const {
    ASSERT(str != NULL);
    uint emptyRun, iRank, iFyle;
    for (iRank = 0; iRank < 8; iRank++) {
        const pieceT* pBoard = &(Board[(7 - iRank) * 8]);
        emptyRun = 0;
        if (iRank > 0) {
            *str++ = '/';
        }
        for (iFyle = 0; iFyle < 8; iFyle++, pBoard++) {
            if (*pBoard != EMPTY) {
                if (emptyRun) {
                    *str++ = (byte)emptyRun + '0';
                }
                emptyRun = 0;
                *str++ = PIECE_CHAR[*pBoard];
            } else {
                emptyRun++;
            }
        }
        if (emptyRun) {
            *str++ = (byte)emptyRun + '0';
        }
    }

    *str++ = ' ';
    *str++ = (ToMove == WHITE ? 'w' : 'b');

    // Add the castling flags and EP flag as well:
    *str++ = ' ';
    const auto no_flags = str;
    if (validCastlingFlag(WHITE, true)) {
        auto rook_sq = castleRookSq(WHITE, true);
        *str++ = isChess960() && rook_sq != find_castle_rook(WHITE, H1)
                     ? toupper(square_FyleChar(rook_sq))
                     : 'K';
    }
    if (validCastlingFlag(WHITE, false)) {
        auto rook_sq = castleRookSq(WHITE, false);
        *str++ = isChess960() && rook_sq != find_castle_rook(WHITE, A1)
                     ? toupper(square_FyleChar(rook_sq))
                     : 'Q';
    }
    if (validCastlingFlag(BLACK, true)) {
        auto rook_sq = castleRookSq(BLACK, true);
        *str++ = isChess960() && rook_sq != find_castle_rook(BLACK, H8)
                     ? square_FyleChar(rook_sq)
                     : 'k';
    }
    if (validCastlingFlag(BLACK, false)) {
        auto rook_sq = castleRookSq(BLACK, false);
        *str++ = isChess960() && rook_sq != find_castle_rook(BLACK, A8)
                     ? square_FyleChar(rook_sq)
                     : 'q';
    }
    if (str == no_flags) {
        *str++ = '-';
    }
    *str++ = ' ';

    // Now the EP target square:
    if (EPTarget == NULL_SQUARE) {
        *str++ = '-';
    } else {
        *str++ = square_FyleChar(EPTarget);
        *str++ = square_RankChar(EPTarget);
    }

    // Also print the Halfmove and ply counters:
    *str++ = ' ';
    sprintf(str, "%d %d", HalfMoveClock, (PlyCounter / 2) + 1);
}
