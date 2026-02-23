#include "cbh_position.h"
#include <cstring>

static byte const PieceMap[16] = {
    EMPTY, WK, WQ, WN, WB, WR, WP, EMPTY, EMPTY, BK, BQ, BN, BB, BR, BP, EMPTY,
};

namespace decoder {

PositionStack::PositionStack() { stack_.push(Lookup()); }

void PositionStack::reset() {
	ASSERT(!stack_.empty());

	while (stack_.size() > 1)
		stack_.pop();
}

unsigned PositionStack::variationLevel() const { return stack_.size() - 1; }

void PositionStack::setup(const byte* str, bool isChess960) {
	unsigned int l = 0;
	auto readbit = [&]() {
		bool bit = str[l / 8] & (1 << (7 - l % 8));
		// printf("Bit %d: %d\n", l, bit ? 1 : 0);
		l++;
		return bit ? 0x1 : 0x0;
	};
	auto read4bit = [&]() {
		return readbit() << 3 | readbit() << 2 | readbit() << 1 | readbit();
	};
	auto read8bit = [&]() { return read4bit() << 4 | read4bit(); };
	auto read16bit = [&]() { return read8bit() << 8 | read8bit(); };

	reset();

	Lookup& lookup = stack_.top();
	Position& pos = lookup.pos;
	Pieces& pieces = lookup.pieces;
	Count& pieceCount = lookup.pieceCount;

	pos.Clear();
	l += 11;

	colorT sideToMove = readbit();
	pos.SetToMove(sideToMove);

	byte epFyle = read4bit();

	l += 4;

	bool bshrt = readbit();
	bool blong = readbit();
	bool wshrt = readbit();
	bool wlong = readbit();

	pos.SetPlyCounter(std::max(1u, (unsigned int)(read8bit() - 1) * 2) +
	                  sideToMove);

	::memset(pieces, NULL_SQUARE, sizeof(pieces));
	::memset(pieceCount, 0, sizeof(pieceCount));

	byte countPieces[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	byte cbpieces[64];

	for (unsigned i = 0; i < 64; i++) {
		byte sq = mapSquare(i);
		if (readbit()) {
			byte piece = PieceMap[read4bit()];
			ASSERT(piece != EMPTY);
			cbpieces[sq] = piece;
			byte& count = countPieces[piece];

			ASSERT(count < 10);

			pieces[count][piece] = sq;
			pieceCount[sq] = count++;
		} else {
			cbpieces[sq] = EMPTY;
		}
	}

	l = 28 * 8; // set position for reading the last 8 bytes

	/*
	 * The pieces must be added in the same order as in the FEN, since Pos and
	 * PosList for the start position will be determined by its FEN when
	 * decoding from memory buffer
	 */
	for (int row = 7; row >= 0; --row) {
		for (int col = 0; col < 8; ++col) {
			byte square = row * 8 + col;
			byte piece = cbpieces[square];
			if (piece != EMPTY) {
				errorT result = pos.AddPiece(piece, square);
				ASSERT(result == OK);
			}
		}
	}

	if (isChess960) {
		byte whiteKing = mapSquare(read8bit()); // for future use
		byte blackKing = mapSquare(read8bit()); // for future use
		byte WhiteShortCastleRook = mapSquare(read8bit());
		byte WhiteLongCastleRook = mapSquare(read8bit());
		byte BlackShortCastleRook = mapSquare(read8bit());
		byte BlackLongCastleRook = mapSquare(read8bit());
		uint16_t startPosCode = read16bit(); // for future use
		if (bshrt)
			pos.setCastling(BLACK, BlackShortCastleRook);
		if (blong)
			pos.setCastling(BLACK, BlackLongCastleRook);
		if (wshrt)
			pos.setCastling(WHITE, WhiteShortCastleRook);
		if (wlong)
			pos.setCastling(WHITE, WhiteLongCastleRook);
	} else {
		if (bshrt)
			pos.setCastling(BLACK, H8);
		if (blong)
			pos.setCastling(BLACK, A8);
		if (wshrt)
			pos.setCastling(WHITE, H1);
		if (wlong)
			pos.setCastling(WHITE, A1);
	}

	if (epFyle)
		pos.SetEPTarget(
		    square_Make(A_FYLE + (epFyle - 1), sideToMove == WHITE ? 5 : 2));

	// Fix illegal en passant squares

	/*
	printf("\nFEN from starting position: \n");
	char fen[1024];
	pos.PrintFEN(fen);
	printf("%s\n", fen);
	*/
}

void PositionStack::setup() {
#define __ NULL_SQUARE
	static Pieces const StandardPosition = {
	//   e   wk  wq  wr  wb  wn  wp  -   -   bk  bq  br  bb  bn  bp
	    {__, E1, D1, A1, C1, B1, A2, __, __, E8, D8, A8, C8, B8, A7},
	    {__, __, __, H1, F1, G1, B2, __, __, __, __, H8, F8, G8, B7},
	    {__, __, __, __, __, __, C2, __, __, __, __, __, __, __, C7},
	    {__, __, __, __, __, __, D2, __, __, __, __, __, __, __, D7},
	    {__, __, __, __, __, __, E2, __, __, __, __, __, __, __, E7},
	    {__, __, __, __, __, __, F2, __, __, __, __, __, __, __, F7},
	    {__, __, __, __, __, __, G2, __, __, __, __, __, __, __, G7},
	    {__, __, __, __, __, __, H2, __, __, __, __, __, __, __, H7},
	    {__, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
	    {__, __, __, __, __, __, __, __, __, __, __, __, __, __, __},
	};
#undef __
#define _ 0
	static Count const PieceCountSetup = {
	    0, 0, 0, 0, 0, 1, 1, 1, // a1 .. h1
	    0, 1, 2, 3, 4, 5, 6, 7, // a2 .. h2
	    _, _, _, _, _, _, _, _, // a3 .. h3
	    _, _, _, _, _, _, _, _, // a4 .. h4
	    _, _, _, _, _, _, _, _, // a5 .. h5
	    _, _, _, _, _, _, _, _, // a6 .. h6
	    0, 1, 2, 3, 4, 5, 6, 7, // a7 .. h7
	    0, 0, 0, 0, 0, 1, 1, 1, // a8 .. h8
	};
#undef _

	reset();

	Lookup& lookup = stack_.top();

	lookup.pos = Position::getStdStart();
	::memcpy(lookup.pieces, StandardPosition, sizeof(StandardPosition));
	::memcpy(lookup.pieceCount, PieceCountSetup, sizeof(PieceCountSetup));
}

Position const& PositionStack::pos() const { return stack_.top().pos; }

simpleMoveT PositionStack::doNullMove() { return doMove(A1, A1, PAWN); }

simpleMoveT PositionStack::doCastling(fyleT toFyle) {
	byte from, to, offs;

	Lookup& lookup = stack_.top();
	Count& pieceCount = lookup.pieceCount;
	Pieces& pieces = lookup.pieces;
	Position& pos = lookup.pos;
	colorT sideToMove = pos.GetToMove();

	fyleT kingFyle = square_Fyle(pos.GetKingSquare(sideToMove));
	bool isShortCastling = toFyle == G_FYLE;

	offs = byte(toFyle - kingFyle);
	doMove(KING, 0, offs, from, to, false);

	byte rank = square_Rank(to);
	// look up position of all rooks of the same color and determine which is
	// closest to the right/left
	pieceT rookType = piece_Make(sideToMove, ROOK);
	squareT firstRook = pieces[0][rookType];
	squareT secondRook = pieces[1][rookType];
	// fix: there could be more than 2 rooks
	byte rookFrom;
	if (secondRook == NULL_SQUARE || square_Rank(secondRook) != rank) {
		rookFrom = firstRook;
	} else if (square_Rank(firstRook) != rank) {
		rookFrom = secondRook;
	} else if (isShortCastling) {
		rookFrom = ((from < firstRook) &&
		            (firstRook < secondRook || secondRook < from))
		               ? firstRook
		               : secondRook;
	} else {
		rookFrom = ((from > secondRook) &&
		            (secondRook > firstRook || firstRook > from))
		               ? secondRook
		               : firstRook;
	}

	byte rookTo = square_Make(isShortCastling ? F_FYLE : D_FYLE, rank);
	byte rookNum = (rookFrom == firstRook) ? 0 : 1;

	pieces[rookNum][rookType] = rookTo;
	pieceCount[rookTo] = rookNum;

	return doMove(from, from, isShortCastling ? KING : ROOK);
}

simpleMoveT PositionStack::doKingMove(byte offs) {
	byte from, to, captured;
	doMove(KING, 0, offs, from, to, true, &captured);
	return doMove(from, to, EMPTY);
}

simpleMoveT PositionStack::doQueenMove(byte number, byte offs) {
	byte from, to, captured;
	doMove(QUEEN, number, offs, from, to, true, &captured);
	return doMove(from, to, EMPTY);
}

simpleMoveT PositionStack::doRookMove(byte number, byte offs) {
	byte from, to, captured;
	doMove(ROOK, number, offs, from, to, true, &captured);
	return doMove(from, to, EMPTY);
}

simpleMoveT PositionStack::doBishopMove(byte number, byte offs) {
	byte from, to, captured;
	doMove(BISHOP, number, offs, from, to, true, &captured);
	return doMove(from, to, EMPTY);
}

simpleMoveT PositionStack::doKnightMove(byte number, byte offs) {
	byte from, to, captured;
	doMove(KNIGHT, number, offs, from, to, false, &captured);
	return doMove(from, to, EMPTY);
}

simpleMoveT PositionStack::doPawnOneForward(byte number) {
	byte from, to;
	colorT sideToMove = stack_.top().pos.GetToMove();
	doMove(PAWN, number, sideToMove == WHITE ? +8 : -8, from, to, false);
	return doMove(from, to, EMPTY);
}

simpleMoveT PositionStack::doPawnTwoForward(byte number) {
	byte from, to;
	colorT sideToMove = stack_.top().pos.GetToMove();
	doMove(PAWN, number, sideToMove == WHITE ? +16 : -16, from, to, false);
	return doMove(from, to, EMPTY);
}

simpleMoveT PositionStack::doCapture(byte number, byte offs) {
	byte from, to, captured;
	doMove(PAWN, number, offs, from, to, false, &captured);
	if (captured != EMPTY)
		return doMove(from, to, EMPTY);

	Lookup& lookup = stack_.top();
	squareT epSquare = from + ((offs == +9 || offs == byte(-7)) ? 1 : -1);
	pieceT capturedPiece = lookup.pos.GetPiece(epSquare);

	if (capturedPiece == EMPTY) { // Chessbase can write corrupt games with empty e.p. squares
		printf("Illegal move: No piece on en passant square\n");
		return simpleMoveT::empty();
	}

	handleCapture(lookup.pieces, lookup.pieceCount, epSquare, capturedPiece);
	return doMove(from, to, EMPTY);
}

simpleMoveT PositionStack::doCaptureRight(byte number) {
	colorT sideToMove = stack_.top().pos.GetToMove();
	return doCapture(number, sideToMove == WHITE ? +9 : -9);
}
simpleMoveT PositionStack::doCaptureLeft(byte number) {
	colorT sideToMove = stack_.top().pos.GetToMove();
	return doCapture(number, sideToMove == WHITE ? +7 : -7);
}

simpleMoveT PositionStack::doMove(byte from, byte to, byte promoted) {
	Position& pos = stack_.top().pos;
	if (from == NULL_SQUARE || to == NULL_SQUARE) {
		printf("Illegal move: from or to are null squares\n");
		return simpleMoveT::empty();
	}
	simpleMoveT sm;
	pos.makeMove(from, to, promoted, sm);

	if (sm.promote != EMPTY) { // handle promotion
		Lookup& lookup = stack_.top();
		Pieces& pieces = lookup.pieces;
		Count& pieceCount = lookup.pieceCount;
		Position& pos = lookup.pos;
		byte capturedPiece = pos.GetPiece(to);
		unsigned number = 10;

		colorT sideToMove = stack_.top().pos.GetToMove();
		promoted = piece_Make(sideToMove, promoted);

		// the number of the promoted piece is the smallest free one
		for (int i = 0; i < 10; i++) {
			if (pieces[i][promoted] == NULL_SQUARE) {
				number = i;
				break;
			}
		}
		if (number == 10) // shouldn't happen
			return simpleMoveT::empty();

		handleCapture(pieces, pieceCount, to, capturedPiece);
		pieces[number][promoted] = to;
		pieceCount[to] = number;
	}
	// We could check for legality here, but that would decrease the performance
	// very considerably

	pos.DoSimpleMove(sm);
	return sm;
}

void PositionStack::handleCapture(Pieces& pieces, Count& pieceCount, byte to,
                                  byte piece) {
	if (piece == EMPTY)
		return;

	byte pieceNum = pieceCount[to];

	pieces[pieceNum][piece] = NULL_SQUARE;

	if (piece_Type(piece) == PAWN || pieceNum > 1)
		return;

	for (unsigned i = 0; i < 10; ++i) {
		byte square = pieces[i][piece];

		if (square != NULL_SQUARE) {
			byte number = pieceCount[square];

			if (number > pieceNum && number < 3) {
				pieces[number - 1][piece] = pieces[number][piece];
				pieces[number][piece] = NULL_SQUARE;
				--pieceCount[square];
			}
		} else if (i != pieceNum) {
			return;
		}
	}
}

void PositionStack::doMove(pieceT pieceType, byte number, byte offs, byte& from,
                           byte& to, bool dontWrap, byte* captured) {
	Lookup& lookup = stack_.top();
	Pieces& pieces = lookup.pieces;
	Count& pieceCount = lookup.pieceCount;
	Position pos = lookup.pos;

	colorT sideToMove = pos.GetToMove();
	byte& square = pieces[number][pieceType | (sideToMove << 3)];

	if (dontWrap && square_Fyle(square) + (offs & 0x7) > H_FYLE)
		offs -= 8;

	from = square;
	to = (from + offs) & 63;

	if (captured) // if we have a memory address to save the type of a
	              // potentially captured piece to
	{
		pieceT capturedPiece = pos.GetPiece(to);
		handleCapture(pieces, pieceCount, to, capturedPiece);
		*captured = capturedPiece;
	}
	square = to;
	pieceCount[to] = number;
}

simpleMoveT PositionStack::doMultibyteMove(byte from, byte to, byte promote) {
	if (promote != EMPTY) {
		return doMove(from, to, promote);
	}

	Lookup& lookup = stack_.top();
	Pieces& pieces = lookup.pieces;
	Count& pieceCount = lookup.pieceCount;
	Position pos = lookup.pos;

	byte number = pieceCount[from];
	pieceT piece = pos.GetPiece(from);
	pieces[number][piece] = to;
	pieceCount[to] = number;

	return doMove(from, to, promote);
}
} // namespace decoder