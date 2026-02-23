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

#include "cbh_decode_game.h"

constexpr auto GAME_HEADER_SIZE = 26;

// at address 0x011941c0 in ChessProgram18.exe
constexpr byte MoveNumberLookup[256] = {
    0xa2, 0x95, 0x43, 0xf5, 0xc1, 0x3d, 0x4a, 0x6c, //   0 -   7
    0x53, 0x83, 0xcc, 0x7c, 0xff, 0xae, 0x68, 0xad, //   8 -  15
    0xd1, 0x92, 0x8b, 0x8d, 0x35, 0x81, 0x5e, 0x74, //  16 -  23
    0x26, 0x8e, 0xab, 0xca, 0xfd, 0x9a, 0xf3, 0xa0, //  24 -  31
    0xa5, 0x15, 0xfc, 0xb1, 0x1e, 0xed, 0x30, 0xea, //  32 -  39
    0x22, 0xeb, 0xa7, 0xcd, 0x4e, 0x6f, 0x2e, 0x24, //  40 -  47
    0x32, 0x94, 0x41, 0x8c, 0x6e, 0x58, 0x82, 0x50, //  48 -  55
    0xbb, 0x02, 0x8a, 0xd8, 0xfa, 0x60, 0xde, 0x52, //  56 -  63
    0xba, 0x46, 0xac, 0x29, 0x9d, 0xd7, 0xdf, 0x08, //  64 -  71
    0x21, 0x01, 0x66, 0xa3, 0xf1, 0x19, 0x27, 0xb5, //  72 -  79
    0x91, 0xd5, 0x42, 0x0e, 0xb4, 0x4c, 0xd9, 0x18, //  80 -  87
    0x5f, 0xbc, 0x25, 0xa6, 0x96, 0x04, 0x56, 0x6a, //  88 -  95
    0xaa, 0x33, 0x1c, 0x2b, 0x73, 0xf0, 0xdd, 0xa4, //  96 - 103
    0x37, 0xd3, 0xc5, 0x10, 0xbf, 0x5a, 0x23, 0x34, // 104 - 111
    0x75, 0x5b, 0xb8, 0x55, 0xd2, 0x6b, 0x09, 0x3a, // 112 - 119
    0x57, 0x12, 0xb3, 0x77, 0x48, 0x85, 0x9b, 0x0f, // 120 - 127
    0x9e, 0xc7, 0xc8, 0xa1, 0x7f, 0x7a, 0xc0, 0xbd, // 128 - 135
    0x31, 0x6d, 0xf6, 0x3e, 0xc3, 0x11, 0x71, 0xce, // 136 - 143
    0x7d, 0xda, 0xa8, 0x54, 0x90, 0x97, 0x1f, 0x44, // 144 - 151
    0x40, 0x16, 0xc9, 0xe3, 0x2c, 0xcb, 0x84, 0xec, // 152 - 159
    0x9f, 0x3f, 0x5c, 0xe6, 0x76, 0x0b, 0x3c, 0x20, // 160 - 167
    0xb7, 0x36, 0x00, 0xdc, 0xe7, 0xf9, 0x4f, 0xf7, // 168 - 175
    0xaf, 0x06, 0x07, 0xe0, 0x1a, 0x0a, 0xa9, 0x4b, // 176 - 183
    0x0c, 0xd6, 0x63, 0x87, 0x89, 0x1d, 0x13, 0x1b, // 184 - 191
    0xe4, 0x70, 0x05, 0x47, 0x67, 0x7b, 0x2f, 0xee, // 192 - 199
    0xe2, 0xe8, 0x98, 0x0d, 0xef, 0xcf, 0xc4, 0xf4, // 200 - 207
    0xfb, 0xb0, 0x17, 0x99, 0x64, 0xf2, 0xd4, 0x2a, // 208 - 215
    0x03, 0x4d, 0x78, 0xc6, 0xfe, 0x65, 0x86, 0x88, // 216 - 223
    0x79, 0x45, 0x3b, 0xe5, 0x49, 0x8f, 0x2d, 0xb9, // 224 - 231
    0xbe, 0x62, 0x93, 0x14, 0xe9, 0xd0, 0x38, 0x9c, // 232 - 239
    0xb2, 0xc2, 0x59, 0x5d, 0xb6, 0x72, 0x51, 0xf8, // 240 - 247
    0x28, 0x7e, 0x61, 0x39, 0xe1, 0xdb, 0x69, 0x80, // 248 - 255
};

// address 0x011941c0 in ChessProgram18.exe
constexpr byte MoveNumberLookup960[256] = {
    0x38, 0x50, 0x64, 0xcc, 0x9f, 0x56, 0x6f, 0x73, //   0 -   7
    0x4d, 0x41, 0x66, 0xa2, 0xc6, 0xb9, 0x96, 0x47, //   8 -  15
    0xee, 0xcf, 0xb1, 0x6d, 0x48, 0x4f, 0x44, 0x20, //  16 -  23
    0x15, 0xbe, 0x82, 0x09, 0x34, 0x99, 0xeb, 0x1a, //  24 -  31
    0x33, 0x7c, 0xab, 0x59, 0x00, 0x22, 0x24, 0xb0, //  32 -  63
    0xb4, 0xfa, 0x85, 0xdd, 0x57, 0xf3, 0xd9, 0xd2, //  40 -  79
    0x8a, 0x8d, 0x70, 0xb5, 0x6e, 0x43, 0xa7, 0x0b, //  48 -  55
    0x27, 0x08, 0xea, 0xb3, 0x94, 0x1c, 0x63, 0x1b, //  56 -  63
    0x2b, 0xc5, 0x0c, 0xd4, 0x3f, 0x37, 0xe8, 0xde, //  64 -  71
    0xa6, 0x23, 0xef, 0x06, 0xfd, 0x3e, 0xc1, 0x40, //  72 -  79
    0x32, 0xdf, 0x2d, 0xc8, 0xd0, 0x0f, 0xf1, 0xe2, //  80 -  87
    0x7a, 0x95, 0xec, 0x3c, 0xca, 0x49, 0x11, 0xe0, //  88 -  95
    0x25, 0xd8, 0x30, 0xfe, 0x2e, 0xba, 0xdc, 0xaa, //  96 - 103
    0x53, 0x4c, 0xd1, 0x5e, 0x67, 0x8b, 0x0d, 0x9e, // 104 - 111
    0xa9, 0xbf, 0x05, 0x78, 0x35, 0xc4, 0xb6, 0xa4, // 112 - 119
    0x4b, 0xd6, 0x04, 0x52, 0xa8, 0x76, 0x2c, 0xbd, // 120 - 127
    0xae, 0x39, 0xb2, 0xe3, 0x03, 0x81, 0x17, 0x9a, // 128 - 135
    0x3d, 0x68, 0xc0, 0x65, 0x92, 0xc9, 0xf7, 0x77, // 136 - 143
    0x16, 0xff, 0xf5, 0xe1, 0x1e, 0x36, 0xbb, 0x86, // 144 - 151
    0x8e, 0xf9, 0x9d, 0xd5, 0x6b, 0xf4, 0x74, 0x80, // 152 - 159
    0x2f, 0x7b, 0x14, 0x97, 0x29, 0xe5, 0x4a, 0x02, // 160 - 167
    0x93, 0x5c, 0xcb, 0xa1, 0x58, 0x79, 0xf8, 0x5f, // 168 - 175
    0x26, 0x28, 0x8f, 0x7e, 0xa5, 0xed, 0xd3, 0x89, // 176 - 183
    0xc7, 0x88, 0x9b, 0x54, 0x87, 0x60, 0xda, 0xf0, // 184 - 191
    0x45, 0x3a, 0xc2, 0x84, 0x90, 0x21, 0xf6, 0xf2, // 192 - 199
    0x5a, 0x2a, 0x55, 0x18, 0x61, 0x01, 0xfb, 0xe6, // 200 - 207
    0x8c, 0x6a, 0x12, 0xb8, 0x62, 0x83, 0x98, 0x10, // 208 - 215
    0x71, 0x5b, 0xe4, 0xad, 0xdb, 0x42, 0x0a, 0xcd, // 216 - 223
    0x46, 0xd7, 0x72, 0x07, 0xce, 0x19, 0xb7, 0x3b, // 224 - 231
    0x31, 0x9c, 0x51, 0xc3, 0x69, 0xa3, 0x91, 0xe9, // 232 - 239
    0x0e, 0x1f, 0xfc, 0x75, 0x6c, 0x1d, 0x7d, 0xe7, // 240 - 247
    0xaf, 0x5d, 0xa0, 0xbc, 0x13, 0x7f, 0xac, 0x4e, // 248 - 257
};

CbhGameDecoder::CbhGameDecoder(const char* gameFilename,
                               const char* annotationFilename, fileModeT fmode)
    : CbhDecoder(gameFilename, fmode),
      annotationDecoder(CbhAnnotationDecoder(annotationFilename, fmode)) {}

errorT CbhGameDecoder::open() {
	if (auto err = CbhDecoder::open(); err != OK)
		return err;

	return annotationDecoder.open();
}

errorT CbhGameDecoder::flush() {
	if (auto err = CbhDecoder::flush(); err != OK)
		return err;
	return annotationDecoder.flush();
}

errorT CbhGameDecoder::decode_header() {
	if (auto err = stream_.open(filename_, fmode_))
		return err;

	return annotationDecoder.decode_header();
}

errorT CbhGameDecoder::decode_record(Game& game,
                                     std::vector<uint32_t> offsets) {
	stream_.pubseekpos(offsets[0]);
	if (auto err = startDecoding(game); err != OK)
		return err;

	if (auto err = annotationDecoder.decode_record(game, {offsets[1]}))
		return err;

	auto res = decodeMoves(game);
	if (res == -1)
		printf("Game at offset %d contains illegal moves\n", offsets[0]);
	return OK;
}

errorT CbhGameDecoder::startDecoding(Game& game) {
	char c[4];
	stream_.sgetn(c, 4);
	this->bytes_read_ = 4;

	/*
	 * There are a few games in cbh databases, which cannot be decoded like
	 * the others. They can be identified via their start byte.
	 */

	bool cannot_decode = c[0] & 0x05;
	if (cannot_decode)
		return ERROR_Decode;

	// Read bit 30 and set start position if that bit is set
	bool start_pos = c[0] & 0x40;
	this->is_chess960 = c[0] & 0x0a;

	this->lookup = this->is_chess960 ? MoveNumberLookup960 : MoveNumberLookup;

	this->bytes_total_ = c[1] << 16 | c[2] << 8 | c[3];

	if (start_pos) {
		const int size = this->is_chess960 ? 36 : 28;
		char pos[size];
		stream_.sgetn(pos, size);
		this->bytes_read_ += size;
		position_.setup(reinterpret_cast<byte*>(pos), this->is_chess960);
		Position position = position_.pos();
		game.SetStartPos(position);
	} else {
		position_.setup();
	}
	return OK;
}

uint32_t CbhGameDecoder::decodeMoves(Game& game, uint32_t move_number) {
	simpleMoveT sm;

	// We can't blindly assume that the game ends in a pop
	while (bytes_read_ < bytes_total_) {
		// Read byte from game stream and calculate its move code
		char c[1];
		stream_.sgetn(c, 1);
		bytes_read_ += 1;
		byte b = static_cast<byte>(c[0]);
		byte move_code = this->translate_byte(b, move_number);

		switch (decodeMove(sm, move_code, move_number)) {
		case Token_Move:
			if (sm.isEmpty()) {
				printf("Aborting on move number %d\n", move_number);
				return -1;
			}
			game.AddMove(sm);
			annotationDecoder.addAnnotations(game, move_number);
			move_number++;
			break;
		case Token_Push: {
			// printf("Variation start\n");
			auto location = game.currentLocation();
			move_number = decodeMoves(game, move_number);
			if (move_number == -1)
				return -1;
			game.restoreLocation(location);
			game.MoveForward();
			game.AddVariation();
			break;
		}
		case Token_Pop:
			/*
			 * printf("\nFEN at variation end: \n");
			 *  char str[1024];
			 *  game.GetCurrentPos()->PrintFEN(str);
			 * printf("%s\n", str);
			 */
			return move_number;
		case Token_Skip:
			break;
		}
	}
	return move_number;
}

byte CbhGameDecoder::translate_byte(byte b, int count) {
	if (this->is_chess960) {
		return byte(this->lookup[b] - count);
	}
	return this->lookup[byte(b - count)];
};

uint32_t CbhGameDecoder::decodeMove(simpleMoveT& sm, byte move_code,
                                    uint32_t move_number) {
	switch (move_code) {
#define OFFSET(x, y) ((x) + (y) * 8)

	// Null move ###########################
	case 0x00:
		sm = position_.doNullMove();
		break;

	// King ################################
	case 0x01:
		sm = position_.doKingMove(OFFSET(0, 1));
		break;
	case 0x02:
		sm = position_.doKingMove(OFFSET(1, 1));
		break;
	case 0x03:
		sm = position_.doKingMove(OFFSET(1, 0));
		break;
	case 0x04:
		sm = position_.doKingMove(OFFSET(1, 7));
		break;
	case 0x05:
		sm = position_.doKingMove(OFFSET(0, 7));
		break;
	case 0x06:
		sm = position_.doKingMove(OFFSET(7, 7));
		break;
	case 0x07:
		sm = position_.doKingMove(OFFSET(7, 0));
		break;
	case 0x08:
		sm = position_.doKingMove(OFFSET(7, 1));
		break;
	case 0x09:
		sm = position_.doCastling(G_FYLE);
		break;
	case 0x0a:
		sm = position_.doCastling(C_FYLE);
		break;

	// First Queen #########################
	case 0x0b:
		sm = position_.doQueenMove(0, OFFSET(0, 1));
		break;
	case 0x0c:
		sm = position_.doQueenMove(0, OFFSET(0, 2));
		break;
	case 0x0d:
		sm = position_.doQueenMove(0, OFFSET(0, 3));
		break;
	case 0x0e:
		sm = position_.doQueenMove(0, OFFSET(0, 4));
		break;
	case 0x0f:
		sm = position_.doQueenMove(0, OFFSET(0, 5));
		break;
	case 0x10:
		sm = position_.doQueenMove(0, OFFSET(0, 6));
		break;
	case 0x11:
		sm = position_.doQueenMove(0, OFFSET(0, 7));
		break;
	case 0x12:
		sm = position_.doQueenMove(0, OFFSET(1, 0));
		break;
	case 0x13:
		sm = position_.doQueenMove(0, OFFSET(2, 0));
		break;
	case 0x14:
		sm = position_.doQueenMove(0, OFFSET(3, 0));
		break;
	case 0x15:
		sm = position_.doQueenMove(0, OFFSET(4, 0));
		break;
	case 0x16:
		sm = position_.doQueenMove(0, OFFSET(5, 0));
		break;
	case 0x17:
		sm = position_.doQueenMove(0, OFFSET(6, 0));
		break;
	case 0x18:
		sm = position_.doQueenMove(0, OFFSET(7, 0));
		break;
	case 0x19:
		sm = position_.doQueenMove(0, OFFSET(1, 1));
		break;
	case 0x1a:
		sm = position_.doQueenMove(0, OFFSET(2, 2));
		break;
	case 0x1b:
		sm = position_.doQueenMove(0, OFFSET(3, 3));
		break;
	case 0x1c:
		sm = position_.doQueenMove(0, OFFSET(4, 4));
		break;
	case 0x1d:
		sm = position_.doQueenMove(0, OFFSET(5, 5));
		break;
	case 0x1e:
		sm = position_.doQueenMove(0, OFFSET(6, 6));
		break;
	case 0x1f:
		sm = position_.doQueenMove(0, OFFSET(7, 7));
		break;
	case 0x20:
		sm = position_.doQueenMove(0, OFFSET(1, 7));
		break;
	case 0x21:
		sm = position_.doQueenMove(0, OFFSET(2, 6));
		break;
	case 0x22:
		sm = position_.doQueenMove(0, OFFSET(3, 5));
		break;
	case 0x23:
		sm = position_.doQueenMove(0, OFFSET(4, 4));
		break;
	case 0x24:
		sm = position_.doQueenMove(0, OFFSET(5, 3));
		break;
	case 0x25:
		sm = position_.doQueenMove(0, OFFSET(6, 2));
		break;
	case 0x26:
		sm = position_.doQueenMove(0, OFFSET(7, 1));
		break;

	// First Rook ##########################
	case 0x27:
		sm = position_.doRookMove(0, OFFSET(0, 1));
		break;
	case 0x28:
		sm = position_.doRookMove(0, OFFSET(0, 2));
		break;
	case 0x29:
		sm = position_.doRookMove(0, OFFSET(0, 3));
		break;
	case 0x2a:
		sm = position_.doRookMove(0, OFFSET(0, 4));
		break;
	case 0x2b:
		sm = position_.doRookMove(0, OFFSET(0, 5));
		break;
	case 0x2c:
		sm = position_.doRookMove(0, OFFSET(0, 6));
		break;
	case 0x2d:
		sm = position_.doRookMove(0, OFFSET(0, 7));
		break;
	case 0x2e:
		sm = position_.doRookMove(0, OFFSET(1, 0));
		break;
	case 0x2f:
		sm = position_.doRookMove(0, OFFSET(2, 0));
		break;
	case 0x30:
		sm = position_.doRookMove(0, OFFSET(3, 0));
		break;
	case 0x31:
		sm = position_.doRookMove(0, OFFSET(4, 0));
		break;
	case 0x32:
		sm = position_.doRookMove(0, OFFSET(5, 0));
		break;
	case 0x33:
		sm = position_.doRookMove(0, OFFSET(6, 0));
		break;
	case 0x34:
		sm = position_.doRookMove(0, OFFSET(7, 0));
		break;

	// Second Rook #########################
	case 0x35:
		sm = position_.doRookMove(1, OFFSET(0, 1));
		break;
	case 0x36:
		sm = position_.doRookMove(1, OFFSET(0, 2));
		break;
	case 0x37:
		sm = position_.doRookMove(1, OFFSET(0, 3));
		break;
	case 0x38:
		sm = position_.doRookMove(1, OFFSET(0, 4));
		break;
	case 0x39:
		sm = position_.doRookMove(1, OFFSET(0, 5));
		break;
	case 0x3a:
		sm = position_.doRookMove(1, OFFSET(0, 6));
		break;
	case 0x3b:
		sm = position_.doRookMove(1, OFFSET(0, 7));
		break;
	case 0x3c:
		sm = position_.doRookMove(1, OFFSET(1, 0));
		break;
	case 0x3d:
		sm = position_.doRookMove(1, OFFSET(2, 0));
		break;
	case 0x3e:
		sm = position_.doRookMove(1, OFFSET(3, 0));
		break;
	case 0x3f:
		sm = position_.doRookMove(1, OFFSET(4, 0));
		break;
	case 0x40:
		sm = position_.doRookMove(1, OFFSET(5, 0));
		break;
	case 0x41:
		sm = position_.doRookMove(1, OFFSET(6, 0));
		break;
	case 0x42:
		sm = position_.doRookMove(1, OFFSET(7, 0));
		break;

	// First Bishop ########################
	case 0x43:
		sm = position_.doBishopMove(0, OFFSET(1, 1));
		break;
	case 0x44:
		sm = position_.doBishopMove(0, OFFSET(2, 2));
		break;
	case 0x45:
		sm = position_.doBishopMove(0, OFFSET(3, 3));
		break;
	case 0x46:
		sm = position_.doBishopMove(0, OFFSET(4, 4));
		break;
	case 0x47:
		sm = position_.doBishopMove(0, OFFSET(5, 5));
		break;
	case 0x48:
		sm = position_.doBishopMove(0, OFFSET(6, 6));
		break;
	case 0x49:
		sm = position_.doBishopMove(0, OFFSET(7, 7));
		break;
	case 0x4a:
		sm = position_.doBishopMove(0, OFFSET(1, 7));
		break;
	case 0x4b:
		sm = position_.doBishopMove(0, OFFSET(2, 6));
		break;
	case 0x4c:
		sm = position_.doBishopMove(0, OFFSET(3, 5));
		break;
	case 0x4d:
		sm = position_.doBishopMove(0, OFFSET(4, 4));
		break;
	case 0x4e:
		sm = position_.doBishopMove(0, OFFSET(5, 3));
		break;
	case 0x4f:
		sm = position_.doBishopMove(0, OFFSET(6, 2));
		break;
	case 0x50:
		sm = position_.doBishopMove(0, OFFSET(7, 1));
		break;

	// Second Bishop #######################
	case 0x51:
		sm = position_.doBishopMove(1, OFFSET(1, 1));
		break;
	case 0x52:
		sm = position_.doBishopMove(1, OFFSET(2, 2));
		break;
	case 0x53:
		sm = position_.doBishopMove(1, OFFSET(3, 3));
		break;
	case 0x54:
		sm = position_.doBishopMove(1, OFFSET(4, 4));
		break;
	case 0x55:
		sm = position_.doBishopMove(1, OFFSET(5, 5));
		break;
	case 0x56:
		sm = position_.doBishopMove(1, OFFSET(6, 6));
		break;
	case 0x57:
		sm = position_.doBishopMove(1, OFFSET(7, 7));
		break;
	case 0x58:
		sm = position_.doBishopMove(1, OFFSET(1, 7));
		break;
	case 0x59:
		sm = position_.doBishopMove(1, OFFSET(2, 6));
		break;
	case 0x5a:
		sm = position_.doBishopMove(1, OFFSET(3, 5));
		break;
	case 0x5b:
		sm = position_.doBishopMove(1, OFFSET(4, 4));
		break;
	case 0x5c:
		sm = position_.doBishopMove(1, OFFSET(5, 3));
		break;
	case 0x5d:
		sm = position_.doBishopMove(1, OFFSET(6, 2));
		break;
	case 0x5e:
		sm = position_.doBishopMove(1, OFFSET(7, 1));
		break;

	// First Knight ########################
	case 0x5f:
		sm = position_.doKnightMove(0, OFFSET(+2, +1));
		break;
	case 0x60:
		sm = position_.doKnightMove(0, OFFSET(+1, +2));
		break;
	case 0x61:
		sm = position_.doKnightMove(0, OFFSET(-1, +2));
		break;
	case 0x62:
		sm = position_.doKnightMove(0, OFFSET(-2, +1));
		break;
	case 0x63:
		sm = position_.doKnightMove(0, OFFSET(-2, -1));
		break;
	case 0x64:
		sm = position_.doKnightMove(0, OFFSET(-1, -2));
		break;
	case 0x65:
		sm = position_.doKnightMove(0, OFFSET(+1, -2));
		break;
	case 0x66:
		sm = position_.doKnightMove(0, OFFSET(+2, -1));
		break;

	// Second Knight #######################
	case 0x67:
		sm = position_.doKnightMove(1, OFFSET(+2, +1));
		break;
	case 0x68:
		sm = position_.doKnightMove(1, OFFSET(+1, +2));
		break;
	case 0x69:
		sm = position_.doKnightMove(1, OFFSET(-1, +2));
		break;
	case 0x6a:
		sm = position_.doKnightMove(1, OFFSET(-2, +1));
		break;
	case 0x6b:
		sm = position_.doKnightMove(1, OFFSET(-2, -1));
		break;
	case 0x6c:
		sm = position_.doKnightMove(1, OFFSET(-1, -2));
		break;
	case 0x6d:
		sm = position_.doKnightMove(1, OFFSET(+1, -2));
		break;
	case 0x6e:
		sm = position_.doKnightMove(1, OFFSET(+2, -1));
		break;

	// a2/a7 Pawn ##########################
	case 0x6f:
		sm = position_.doPawnOneForward(0);
		break;
	case 0x70:
		sm = position_.doPawnTwoForward(0);
		break;
	case 0x71:
		sm = position_.doCaptureRight(0);
		break;
	case 0x72:
		sm = position_.doCaptureLeft(0);
		break;

	// b2/b7 Pawn ##########################
	case 0x73:
		sm = position_.doPawnOneForward(1);
		break;
	case 0x74:
		sm = position_.doPawnTwoForward(1);
		break;
	case 0x75:
		sm = position_.doCaptureRight(1);
		break;
	case 0x76:
		sm = position_.doCaptureLeft(1);
		break;

	// c2/c7 Pawn ##########################
	case 0x77:
		sm = position_.doPawnOneForward(2);
		break;
	case 0x78:
		sm = position_.doPawnTwoForward(2);
		break;
	case 0x79:
		sm = position_.doCaptureRight(2);
		break;
	case 0x7a:
		sm = position_.doCaptureLeft(2);
		break;

	// d2/d7 Pawn ##########################
	case 0x7b:
		sm = position_.doPawnOneForward(3);
		break;
	case 0x7c:
		sm = position_.doPawnTwoForward(3);
		break;
	case 0x7d:
		sm = position_.doCaptureRight(3);
		break;
	case 0x7e:
		sm = position_.doCaptureLeft(3);
		break;

	// e2/e7 Pawn ##########################
	case 0x7f:
		sm = position_.doPawnOneForward(4);
		break;
	case 0x80:
		sm = position_.doPawnTwoForward(4);
		break;
	case 0x81:
		sm = position_.doCaptureRight(4);
		break;
	case 0x82:
		sm = position_.doCaptureLeft(4);
		break;

	// f2/f7 Pawn ##########################
	case 0x83:
		sm = position_.doPawnOneForward(5);
		break;
	case 0x84:
		sm = position_.doPawnTwoForward(5);
		break;
	case 0x85:
		sm = position_.doCaptureRight(5);
		break;
	case 0x86:
		sm = position_.doCaptureLeft(5);
		break;

	// g2/g7 Pawn ##########################
	case 0x87:
		sm = position_.doPawnOneForward(6);
		break;
	case 0x88:
		sm = position_.doPawnTwoForward(6);
		break;
	case 0x89:
		sm = position_.doCaptureRight(6);
		break;
	case 0x8a:
		sm = position_.doCaptureLeft(6);
		break;

	// h2/h7 Pawn ##########################
	case 0x8b:
		sm = position_.doPawnOneForward(7);
		break;
	case 0x8c:
		sm = position_.doPawnTwoForward(7);
		break;
	case 0x8d:
		sm = position_.doCaptureRight(7);
		break;
	case 0x8e:
		sm = position_.doCaptureLeft(7);
		break;

	// Second Queen #########################
	case 0x8f:
		sm = position_.doQueenMove(1, OFFSET(0, 1));
		break;
	case 0x90:
		sm = position_.doQueenMove(1, OFFSET(0, 2));
		break;
	case 0x91:
		sm = position_.doQueenMove(1, OFFSET(0, 3));
		break;
	case 0x92:
		sm = position_.doQueenMove(1, OFFSET(0, 4));
		break;
	case 0x93:
		sm = position_.doQueenMove(1, OFFSET(0, 5));
		break;
	case 0x94:
		sm = position_.doQueenMove(1, OFFSET(0, 6));
		break;
	case 0x95:
		sm = position_.doQueenMove(1, OFFSET(0, 7));
		break;
	case 0x96:
		sm = position_.doQueenMove(1, OFFSET(1, 0));
		break;
	case 0x97:
		sm = position_.doQueenMove(1, OFFSET(2, 0));
		break;
	case 0x98:
		sm = position_.doQueenMove(1, OFFSET(3, 0));
		break;
	case 0x99:
		sm = position_.doQueenMove(1, OFFSET(4, 0));
		break;
	case 0x9a:
		sm = position_.doQueenMove(1, OFFSET(5, 0));
		break;
	case 0x9b:
		sm = position_.doQueenMove(1, OFFSET(6, 0));
		break;
	case 0x9c:
		sm = position_.doQueenMove(1, OFFSET(7, 0));
		break;
	case 0x9d:
		sm = position_.doQueenMove(1, OFFSET(1, 1));
		break;
	case 0x9e:
		sm = position_.doQueenMove(1, OFFSET(2, 2));
		break;
	case 0x9f:
		sm = position_.doQueenMove(1, OFFSET(3, 3));
		break;
	case 0xa0:
		sm = position_.doQueenMove(1, OFFSET(4, 4));
		break;
	case 0xa1:
		sm = position_.doQueenMove(1, OFFSET(5, 5));
		break;
	case 0xa2:
		sm = position_.doQueenMove(1, OFFSET(6, 6));
		break;
	case 0xa3:
		sm = position_.doQueenMove(1, OFFSET(7, 7));
		break;
	case 0xa4:
		sm = position_.doQueenMove(1, OFFSET(1, 7));
		break;
	case 0xa5:
		sm = position_.doQueenMove(1, OFFSET(2, 6));
		break;
	case 0xa6:
		sm = position_.doQueenMove(1, OFFSET(3, 5));
		break;
	case 0xa7:
		sm = position_.doQueenMove(1, OFFSET(4, 4));
		break;
	case 0xa8:
		sm = position_.doQueenMove(1, OFFSET(5, 3));
		break;
	case 0xa9:
		sm = position_.doQueenMove(1, OFFSET(6, 2));
		break;
	case 0xaa:
		sm = position_.doQueenMove(1, OFFSET(7, 1));
		break;

	// Third Queen ##########################
	case 0xab:
		sm = position_.doQueenMove(2, OFFSET(0, 1));
		break;
	case 0xac:
		sm = position_.doQueenMove(2, OFFSET(0, 2));
		break;
	case 0xad:
		sm = position_.doQueenMove(2, OFFSET(0, 3));
		break;
	case 0xae:
		sm = position_.doQueenMove(2, OFFSET(0, 4));
		break;
	case 0xaf:
		sm = position_.doQueenMove(2, OFFSET(0, 5));
		break;
	case 0xb0:
		sm = position_.doQueenMove(2, OFFSET(0, 6));
		break;
	case 0xb1:
		sm = position_.doQueenMove(2, OFFSET(0, 7));
		break;
	case 0xb2:
		sm = position_.doQueenMove(2, OFFSET(1, 0));
		break;
	case 0xb3:
		sm = position_.doQueenMove(2, OFFSET(2, 0));
		break;
	case 0xb4:
		sm = position_.doQueenMove(2, OFFSET(3, 0));
		break;
	case 0xb5:
		sm = position_.doQueenMove(2, OFFSET(4, 0));
		break;
	case 0xb6:
		sm = position_.doQueenMove(2, OFFSET(5, 0));
		break;
	case 0xb7:
		sm = position_.doQueenMove(2, OFFSET(6, 0));
		break;
	case 0xb8:
		sm = position_.doQueenMove(2, OFFSET(7, 0));
		break;
	case 0xb9:
		sm = position_.doQueenMove(2, OFFSET(1, 1));
		break;
	case 0xba:
		sm = position_.doQueenMove(2, OFFSET(2, 2));
		break;
	case 0xbb:
		sm = position_.doQueenMove(2, OFFSET(3, 3));
		break;
	case 0xbc:
		sm = position_.doQueenMove(2, OFFSET(4, 4));
		break;
	case 0xbd:
		sm = position_.doQueenMove(2, OFFSET(5, 5));
		break;
	case 0xbe:
		sm = position_.doQueenMove(2, OFFSET(6, 6));
		break;
	case 0xbf:
		sm = position_.doQueenMove(2, OFFSET(7, 7));
		break;
	case 0xc0:
		sm = position_.doQueenMove(2, OFFSET(1, 7));
		break;
	case 0xc1:
		sm = position_.doQueenMove(2, OFFSET(2, 6));
		break;
	case 0xc2:
		sm = position_.doQueenMove(2, OFFSET(3, 5));
		break;
	case 0xc3:
		sm = position_.doQueenMove(2, OFFSET(4, 4));
		break;
	case 0xc4:
		sm = position_.doQueenMove(2, OFFSET(5, 3));
		break;
	case 0xc5:
		sm = position_.doQueenMove(2, OFFSET(6, 2));
		break;
	case 0xc6:
		sm = position_.doQueenMove(2, OFFSET(7, 1));
		break;

	// Third Rook ##########################
	case 0xc7:
		sm = position_.doRookMove(2, OFFSET(0, 1));
		break;
	case 0xc8:
		sm = position_.doRookMove(2, OFFSET(0, 2));
		break;
	case 0xc9:
		sm = position_.doRookMove(2, OFFSET(0, 3));
		break;
	case 0xca:
		sm = position_.doRookMove(2, OFFSET(0, 4));
		break;
	case 0xcb:
		sm = position_.doRookMove(2, OFFSET(0, 5));
		break;
	case 0xcc:
		sm = position_.doRookMove(2, OFFSET(0, 6));
		break;
	case 0xcd:
		sm = position_.doRookMove(2, OFFSET(0, 7));
		break;
	case 0xce:
		sm = position_.doRookMove(2, OFFSET(1, 0));
		break;
	case 0xcf:
		sm = position_.doRookMove(2, OFFSET(2, 0));
		break;
	case 0xd0:
		sm = position_.doRookMove(2, OFFSET(3, 0));
		break;
	case 0xd1:
		sm = position_.doRookMove(2, OFFSET(4, 0));
		break;
	case 0xd2:
		sm = position_.doRookMove(2, OFFSET(5, 0));
		break;
	case 0xd3:
		sm = position_.doRookMove(2, OFFSET(6, 0));
		break;
	case 0xd4:
		sm = position_.doRookMove(2, OFFSET(7, 0));
		break;

	// Third Bishop ########################
	case 0xd5:
		sm = position_.doBishopMove(2, OFFSET(1, 1));
		break;
	case 0xd6:
		sm = position_.doBishopMove(2, OFFSET(2, 2));
		break;
	case 0xd7:
		sm = position_.doBishopMove(2, OFFSET(3, 3));
		break;
	case 0xd8:
		sm = position_.doBishopMove(2, OFFSET(4, 4));
		break;
	case 0xd9:
		sm = position_.doBishopMove(2, OFFSET(5, 5));
		break;
	case 0xda:
		sm = position_.doBishopMove(2, OFFSET(6, 6));
		break;
	case 0xdb:
		sm = position_.doBishopMove(2, OFFSET(7, 7));
		break;
	case 0xdc:
		sm = position_.doBishopMove(2, OFFSET(1, 7));
		break;
	case 0xdd:
		sm = position_.doBishopMove(2, OFFSET(2, 6));
		break;
	case 0xde:
		sm = position_.doBishopMove(2, OFFSET(3, 5));
		break;
	case 0xdf:
		sm = position_.doBishopMove(2, OFFSET(4, 4));
		break;
	case 0xe0:
		sm = position_.doBishopMove(2, OFFSET(5, 3));
		break;
	case 0xe1:
		sm = position_.doBishopMove(2, OFFSET(6, 2));
		break;
	case 0xe2:
		sm = position_.doBishopMove(2, OFFSET(7, 1));
		break;

	// Third Knight ########################
	case 0xe3:
		sm = position_.doKnightMove(2, OFFSET(+2, +1));
		break;
	case 0xe4:
		sm = position_.doKnightMove(2, OFFSET(+1, +2));
		break;
	case 0xe5:
		sm = position_.doKnightMove(2, OFFSET(-1, +2));
		break;
	case 0xe6:
		sm = position_.doKnightMove(2, OFFSET(-2, +1));
		break;
	case 0xe7:
		sm = position_.doKnightMove(2, OFFSET(-2, -1));
		break;
	case 0xe8:
		sm = position_.doKnightMove(2, OFFSET(-1, -2));
		break;
	case 0xe9:
		sm = position_.doKnightMove(2, OFFSET(+1, -2));
		break;
	case 0xea:
		sm = position_.doKnightMove(2, OFFSET(+2, -1));
		break;

	// Multiple byte move ##################
	case 0xeb: {
		char c[2];
		stream_.sgetn(c, 2);
		bytes_read_ += 2;
		byte b1 = static_cast<byte>(c[0]);
		byte b2 = static_cast<byte>(c[1]);
		uint32_t word = this->translate_byte(b1, move_number) << 8;
		word |= this->translate_byte(b2, move_number);
		byte from = decoder::PositionStack::mapSquare(word & 63);
		byte to = decoder::PositionStack::mapSquare((word >> 6) & 63);

		if (from == to) { // chess960 castling
			sm = position_.doCastling(square_Fyle(to));
		} else {
			pieceT moving_piece = piece_Type(position_.pos().GetPiece(from));
			byte to_rank = to >> 3;
			bool isPromotion = (moving_piece == PAWN) &&
			                   (to_rank == 0 || to_rank == 7);
			byte promote = isPromotion ? ((word >> 12) & 3) + QUEEN : EMPTY;
			sm = position_.doMultibyteMove(from, to, promote);
		}
	} break;

	// Padding #############################
	case 0xec:
		return Token_Skip;

	// Unused ##############################
	case 0xed:
		[[fallthrough]];
	case 0xee:
		[[fallthrough]];
	case 0xef:
		[[fallthrough]];
	case 0xf0:
		[[fallthrough]];
	case 0xf1:
		[[fallthrough]];
	case 0xf2:
		[[fallthrough]];
	case 0xf3:
		[[fallthrough]];
	case 0xf4:
		[[fallthrough]];
	case 0xf5:
		[[fallthrough]];
	case 0xf6:
		[[fallthrough]];
	case 0xf7:
		[[fallthrough]];
	case 0xf8:
		[[fallthrough]];
	case 0xf9:
		[[fallthrough]];
	case 0xfa:
		[[fallthrough]];
	case 0xfb:
		[[fallthrough]];
	case 0xfc:
		[[fallthrough]];
	case 0xfd:
		return Token_Skip;

	// Push position #######################
	case 0xfe:
		position_.push();
		return Token_Push;

	// Pop position ########################
	case 0xff:
		if (position_.variationLevel())
			position_.pop();
		return Token_Pop;

#undef OFFSET
	}

	return Token_Move;
}
