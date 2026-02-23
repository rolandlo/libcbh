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

/** @file
 * Implements a decoder for .cbg files.
 */

#pragma once
#include "cbh_decode_annotation.h"
#include "cbh_decode_base.h"
#include "cbh_position.h"
#include "game.h"
#include <vector>

/**
 * This class implements a CBG game decoder that invokes the appropriate member
 * functions of the associated Game object for each type of CBG token.
 */
class CbhGameDecoder final : public CbhDecoder {

public:
	CbhGameDecoder(const char* gameFilename, const char* annotationFilename,
	               fileModeT fmode);

	errorT open() override;
	errorT flush() override;

	errorT decode_header() override; 
	errorT decode_record(Game& game, std::vector<uint32_t> offsets) override; 

private:
	enum { Token_Move, Token_Push, Token_Pop, Token_Skip };

	CbhAnnotationDecoder annotationDecoder;
	decoder::PositionStack position_;
	bool is_chess960;
	const byte* lookup;
	uint32_t bytes_read_;  // number of bytes read from the current game
	uint32_t bytes_total_; // number of bytes in total for the game

	errorT startDecoding(Game& game);

	uint32_t decodeMoves(Game& game, uint32_t move_number = 0); 
	byte translate_byte(byte b, int count);

	uint32_t decodeMove(simpleMoveT& sm, byte move_code, uint32_t move_number); 
};
