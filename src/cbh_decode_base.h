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
 * Abstract class for decoding files belonging to the CBH-format.
 */

#pragma once
#include "error.h"
#include "filebuf.h"
#include "game.h"
#include <vector>

class CbhDecoder {

public:
	FilebufAppend stream_;
	const char* filename_;
	fileModeT fmode_;

	CbhDecoder(const char* filename, fileModeT fmode)
	    : filename_(filename), fmode_(fmode) {}

	virtual errorT open() { return stream_.open(filename_, fmode_); };
	virtual errorT flush() {
		return (stream_.pubsync() == 0) ? OK : ERROR_FileWrite;
	}
	virtual errorT decode_header() = 0;
	virtual errorT decode_record(Game& game, std::vector<uint32_t> offsets) = 0;
};