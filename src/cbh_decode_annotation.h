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
 * Implements a decoder for .cba files.
 */

#pragma once
#include "cbh_decode_base.h"
#include "game.h"

class CbhAnnotationDecoder final : public CbhDecoder {
	bool finished_;
	uint32_t move_number_;
	uint32_t length_;
	uint32_t readBytes_;

public:
	CbhAnnotationDecoder(const char* filename, fileModeT fmode);
	void decodeSquares(Game& game, const char* content, int length);
	void decodeArrows(Game& game, const char* content, int length);
	void decodeSymbol(Game& game, const byte* content, int length);
	errorT decode_header() override;
	errorT decode_record(Game& game, std::vector<uint32_t> offsets)
	    override; // only starts decoding actually
	void addAnnotations(Game& game, uint32_t move_number);
};
