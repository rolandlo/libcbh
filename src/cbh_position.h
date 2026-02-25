#pragma once

#include "mapping.h"
#include "position.h"
#include <stack>

namespace decoder {
class PositionStack {
public:
	PositionStack();

	unsigned variationLevel() const;

	void setup();
	void setup(const byte* str, bool isChess960);

	inline void push() {
		Lookup lookup = stack_.top();
		stack_.push(lookup);
	}
	inline void pop() { stack_.pop(); }

	Position const& pos() const;

	simpleMoveT doNullMove();
	simpleMoveT doCastling(fyleT toFyle);
	simpleMoveT doKingMove(byte offs);
	simpleMoveT doQueenMove(byte number, byte offs);
	simpleMoveT doRookMove(byte number, byte offs);
	simpleMoveT doBishopMove(byte number, byte offs);
	simpleMoveT doKnightMove(byte number, byte offs);
	simpleMoveT doPawnOneForward(byte number);
	simpleMoveT doPawnTwoForward(byte number);
	simpleMoveT doCaptureRight(byte number);
	simpleMoveT doCaptureLeft(byte number);
	simpleMoveT doMultibyteMove(byte from, byte to, byte promote);
	simpleMoveT doMove(byte from, byte to, byte promoted);

private:
	typedef byte Count[64];
	typedef byte Pieces[10][15];

	struct Lookup {
		Position pos;
		Count pieceCount;
		Pieces pieces;
	};

	typedef std::stack<Lookup> Stack;

	void reset();
	void handleCapture(Pieces& pieces, Count& pieceCount, byte to, byte piece);
	void doMove(pieceT pieceType, byte number, byte offs, byte& from, byte& to,
	            bool dontWrap, byte* captured = 0);
	simpleMoveT doCapture(byte number, byte offs);

	Stack stack_;
};
} // namespace decoder