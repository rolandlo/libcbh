/*
 * Copyright (C) 2026  Roland Lötscher
 *
 * This file is part of SCID (Shane's Chess Information Database).
 *
 * SCID is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * SCID is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SCID. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "cbh_decode_annotation.h"

constexpr int ANNOTATION_HEADER_SIZE = 26;
constexpr int ANNOTATION_ENTRY_SIZE = 62;

static std::string squares[64] = {
    "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", // a-file
	"b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", // b-file
	"c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", // c-file
	"d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", // d-file
	"e1", "e2", "e3", "e4", "e5", "e6", "e7", "e8", // e-file
	"f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", // f-file
	"g1", "g2", "g3", "g4", "g5", "g6", "g7", "g8", // g-file
	"h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8", // h-file
};

static inline std::string squareName(byte sq) {
	if (1 < sq && sq < 65) {
		return squares[sq - 1];
	}
	return "";
}

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

CbhAnnotationDecoder::CbhAnnotationDecoder(const char* filename,
                                           fileModeT fmode)
    : CbhDecoder(filename, fmode) {}

errorT CbhAnnotationDecoder::decode_header() {
	if (auto err = stream_.open(filename_, fmode_))
		return err;

	return OK;
}

void CbhAnnotationDecoder::decodeSquares(Game& game, const char* content,
                                         int length) {
	for (int i = 0; 2 * i < length; i++) {
		std::string col = colorName(content[2 * i]);
		std::string sq = squareName(content[2 * i + 1]);
		std::string annotation = "[%draw full," + sq + "," + col + "]";
		auto& str = game.accessMoveComment();
		str = str + annotation;
	}
}

void CbhAnnotationDecoder::decodeArrows(Game& game, const char* content,
                                        int length) {
	for (int i = 0; 3 * i < length; i++) {
		std::string col = colorName(content[3 * i]);
		std::string sqFrom = squareName(content[3 * i + 1]);
		std::string sqTo = squareName(content[3 * i + 2]);
		std::string annotation = "[%draw arrow," + sqFrom + "," + sqTo + "," +
		                         col + "]";
		auto& str = game.accessMoveComment();
		str = str + annotation;
	}
}

void CbhAnnotationDecoder::decodeSymbol(Game& game, const byte* content,
                                        int length) {
	if (length == 0)
		return;

	colorT whiteToMove = game.currentPos()->WhiteToMove();
#define NAG(code) whiteToMove ? NAG_##code : NAG_Black##code

	switch (content[0]) {
	case 0x01:
		game.AddNag(NAG_GoodMove);
		break;
	case 0x02:
		game.AddNag(NAG_PoorMove);
		break;
	case 0x03:
		game.AddNag(NAG_ExcellentMove);
		break;
	case 0x04:
		game.AddNag(NAG_Blunder);
		break;
	case 0x05:
		game.AddNag(NAG_InterestingMove);
		break;
	case 0x06:
		game.AddNag(NAG_DubiousMove);
		break;
	case 0x08:
		game.AddNag(NAG_OnlyMove);
		break;
	case 0x16:
		game.AddNag(NAG(ZugZwang));
		break;
	}

	if (length == 1)
		return;

	switch (content[1]) {
	case 0x0b:
		game.AddNag(NAG_Equal);
		break;
	case 0x0d:
		game.AddNag(NAG_Unclear);
		break;
	case 0x0e:
		game.AddNag(NAG_WhiteSlight);
		break;
	case 0x0f:
		game.AddNag(NAG_BlackSlight);
		break;
	case 0x10:
		game.AddNag(NAG_WhiteClear);
		break;
	case 0x11:
		game.AddNag(NAG_BlackClear);
		break;
	case 0x12:
		game.AddNag(NAG_WhiteDecisive);
		break;
	case 0x13:
		game.AddNag(NAG_BlackDecisive);
		break;
		//		case 0x20: game.AddNag(NAG(HasAModerateTimeAdvantage)); break;
	case 0x24:
		game.AddNag(NAG_WithInitiative);
		break;
	case 0x28:
		game.AddNag(NAG_WithAttack);
		break;
	case 0x2c:
		game.AddNag(NAG_Compensation);
		break;
	case 0x84:
		game.AddNag(NAG(SlightCounterPlay));
		break;
	case 0x8a:
		game.AddNag(NAG_TimeLimit);
		break;
	case 0x92:
		game.AddNag(NAG_Novelty);
		break;
	}

	if (length == 2)
		return;

	switch (content[2]) {
	case 0x8C:
		game.AddNag(NAG_WithIdea);
		break;
		//		case 0x8D: game.AddNag(NAG_AimedAgainst); break;
	case 0x8E:
		game.AddNag(NAG_BetterIs);
		break;
	case 0x8F:
		game.AddNag(NAG_WorseIs);
		break;
		//		case 0x90: game.AddNag(NAG_EquivalentMove); break;
	case 0x91:
		game.AddNag(NAG_Comment);
		break;
	}
#undef NAG
}

errorT CbhAnnotationDecoder::decode_record(Game& game,
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

void CbhAnnotationDecoder::addAnnotations(Game& game, uint32_t move_number) {
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
			const char* text = content + 2;
			// Prepend comment
			auto& str = game.accessMoveComment();
			str = text + str;
			break;
		}
		case 0x03: // symbol
		{
			decodeSymbol(game, reinterpret_cast<byte*>(content), size);
			break;
		}
		case 0x04: // squares
			decodeSquares(game, content, size);
			break;
		case 0x05: // arrows
			decodeArrows(game, content, size);
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
			const char* text = content + 2;
			// Append comment to previous move
			game.MoveBackup();
			auto& str = game.accessMoveComment();
			str = str + text;
			game.MoveForward();
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
