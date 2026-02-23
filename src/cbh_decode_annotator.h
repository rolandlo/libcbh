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
 * Implements a decoder for .cbc files.
 */

#pragma once
#include "cbh_decode_base.h"
#include "game.h"

class CbhAnnotatorDecoder final : public CbhDecoder {
	size_t annotator_header_size_;

public:
	CbhAnnotatorDecoder(const char* filename, fileModeT fmode);

	errorT decode_header() override;
	errorT decode_record(Game& game, std::vector<uint32_t> offsets) override;
};
