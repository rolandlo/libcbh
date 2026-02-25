/*
 * Copyright (C) 2026 Roland Lötscher.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
 * THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "cbh_decode_annotation.h"
#include "mapping.h"
#include "nags.h"

constexpr int ANNOTATION_HEADER_SIZE = 26;
constexpr int ANNOTATION_ENTRY_SIZE = 62;

static inline std::string colorName(byte col) {
	switch (col) {
	case 2:
		return "green";
	case 3:
		return "yellow";
	case 4: 
		[[fallthrough]];
	default:
		return "red";
	}
}

CbhAnnotationDecoder::CbhAnnotationDecoder(const char* filename)
    : CbhDecoder(filename) {}

errorT CbhAnnotationDecoder::decode_header() {
	if (auto err = stream_.open(filename_, FMODE_ReadOnly))
		return err;

	return OK;
}

void CbhAnnotationDecoder::decodeSquares(std::vector<Comment>& comments,
                                         const char* content, int length) {
	for (int i = 0; 2 * i < length; i++) {
		std::string col = colorName(content[2 * i]);
		squareT square = mapSquare(content[2 * i + 1] - 1);
		comments.push_back(SquareComment(square, col));
	}
}

void CbhAnnotationDecoder::decodeArrows(std::vector<Comment>& comments,
                                        const char* content, int length) {
	for (int i = 0; 3 * i < length; i++) {
		std::string col = colorName(content[3 * i]);
		squareT sqFrom = mapSquare(content[3 * i + 1] - 1);
		squareT sqTo = mapSquare(content[3 * i + 2] - 1);
		comments.push_back(ArrowComment(sqFrom, sqTo, col));
	}
}

void CbhAnnotationDecoder::decodeSymbol(std::vector<Comment>& comments,
                                        const byte* content, int length,
                                        colorT sideToMove) {
	if (length == 0)
		return;
	SymbolComment comment;

#define NAG(code) (sideToMove == WHITE) ? NAG_White##code : NAG_Black##code

	switch (content[0]) {
	case 0x01:
		comment.symbol = NAG_GoodMove;
		break;
	case 0x02:
		comment.symbol = NAG_PoorMove;
		break;
	case 0x03:
		comment.symbol = NAG_ExcellentMove;
		break;
	case 0x04:
		comment.symbol = NAG_Blunder;
		break;
	case 0x05:
		comment.symbol = NAG_InterestingMove;
		break;
	case 0x06:
		comment.symbol = NAG_DubiousMove;
		break;
	case 0x08:
		comment.symbol = NAG_OnlyMove;
		break;
	case 0x16:
		comment.symbol = NAG(ZugZwang);
		break;
	}

	if (length == 1) {
		comments.push_back(comment);
		return;
	}

	switch (content[1]) {
	case 0x0b:
		comment.evaluation = NAG_Equal;
		break;
	case 0x0d:
		comment.evaluation = NAG_Unclear;
		break;
	case 0x0e:
		comment.evaluation = NAG_WhiteSlight;
		break;
	case 0x0f:
		comment.evaluation = NAG_BlackSlight;
		break;
	case 0x10:
		comment.evaluation = NAG_WhiteClear;
		break;
	case 0x11:
		comment.evaluation = NAG_BlackClear;
		break;
	case 0x12:
		comment.evaluation = NAG_WhiteDecisive;
		break;
	case 0x13:
		comment.evaluation = NAG_BlackDecisive;
		break;
	case 0x20:
		comment.evaluation = NAG(ModerateDevelopmentAdvantage);
		break;
	case 0x24:
		comment.evaluation = NAG(WithInitiative);
		break;
	case 0x28:
		comment.evaluation = NAG(WithAttack);
		break;
	case 0x2c:
		comment.evaluation = NAG(Compensation);
		break;
	case 0x84:
		comment.evaluation = NAG(CounterPlay);
		break;
	case 0x8a:
		comment.evaluation = NAG(TimeControlPressure);
		break;
	case 0x92:
		comment.evaluation = NAG_Novelty;
		break;
	}

	if (length == 2) {
		comments.push_back(comment);
		return;
	}

	switch (content[2]) {
	case 0x8C:
		comment.prefix = NAG_WithIdea;
		break;
	case 0x8D:
		comment.prefix = NAG_AimedAgainst;
		break;
	case 0x8E:
		comment.prefix = NAG_BetterIs;
		break;
	case 0x8F:
		comment.prefix = NAG_WorseIs;
		break;
	case 0x90:
		comment.prefix = NAG_EquivalentIs;
		break;
	case 0x91:
		comment.prefix = NAG_EditorialComment;
		break;
	}
#undef NAG
	comments.push_back(comment);
}

errorT CbhAnnotationDecoder::decode_record(GameReturnValue& game,
                                           std::vector<uint32_t> offsets) {
	uint32_t annotation_offset = offsets.at(0);
	stream_.pubseekpos(annotation_offset + 10); // move to offset 10

	// Read number of bytes for annotations in this game (4 bytes)
	char ls[4];
	stream_.sgetn(ls, 4);
	length_ = static_cast<byte>(ls[0]) << 24 | static_cast<byte>(ls[1]) << 16 |
	          static_cast<byte>(ls[2]) << 8 | static_cast<byte>(ls[3]);

	readBytes_ = 14;

	// Read position in game (3 bytes) big endian (mistake on talkchess?)
	if (readBytes_ < length_) {
		char ps[3];
		stream_.sgetn(ps, 3);
		move_number_ = static_cast<byte>(ps[0]) << 16 |
		               static_cast<byte>(ps[1]) << 8 | static_cast<byte>(ps[2]);
		readBytes_ += 3;
		finished_ = false;
	} else {
		finished_ = true;
	}

	return OK;
}

void CbhAnnotationDecoder::addAnnotations(std::vector<Comment>& comments,
                                          uint32_t move_number,
                                          colorT sideToMove) {
	if (finished_ || move_number != move_number_) {
		return;
	}

	while (readBytes_ < length_ && move_number_ == move_number) {

		// Read type of annotation (1 byte)
		byte type = static_cast<byte>(stream_.sbumpc());

		// Read length of annotation (2 bytes, little endian) including 6
		// previous bytes
		char al[2];
		stream_.sgetn(al, 2);
		uint16_t size =
		    (static_cast<byte>(al[0]) << 8 | static_cast<byte>(al[1])) - 6;

		char content[size + 1];
		stream_.sgetn(content, size);
		content[size] = 0;
		readBytes_ += size + 3;

		switch (type) {
		case 0x02: // text after move
		{
			langT lang = static_cast<byte>(content[0]) << 8 |
			             static_cast<byte>(
			                 content[1]); // big Endian (mistake on talkchess?)
			const char* text = content + 2;
			comments.push_back(TextAfterComment(lang, text));
			break;
		}
		case 0x03: // symbol
		{
			decodeSymbol(comments, reinterpret_cast<byte*>(content), size,
			             sideToMove);
			break;
		}
		case 0x04: // squares
			decodeSquares(comments, content, size);
			break;
		case 0x05: // arrows
			decodeArrows(comments, content, size);
			break;
		case 0x09: // training annotation
			break;
		case 0x10: // sound
			break;
		case 0x11: // picture
			break;
		case 0x13: // game quotation
			break;
		case 0x14: // pawn structure
			break;
		case 0x15: // piece path
			break;
		case 0x18: // critical position
			break;
		case 0x19: // correspondence move
			break;
		case 0x22: // medal
			break;
		case 0x23: // variation color
			break;
		case 0x24: // Time control
			break;
		case 0x61: // correspondence header
			break;
		case 0x82: // text before move
		{
			langT lang = static_cast<byte>(content[0]) << 8 |
			             static_cast<byte>(
			                 content[1]); // big Endian (mistake on talkchess?)
			const char* text = content + 2;
			comments.push_back(TextBeforeComment(lang, text));
			break;
		}
		default: // unknown annotation type
			break;
		}

		if (readBytes_ < length_) {
			// Read position in game (3 bytes) big endian (mistake on
			// talkchess?)
			char ps[3];
			stream_.sgetn(ps, 3);
			move_number_ = static_cast<byte>(ps[0]) << 16 |
			               static_cast<byte>(ps[1]) << 8 |
			               static_cast<byte>(ps[2]);
			readBytes_ += 3;
		} else {
			finished_ = true;
		}
	}
}
