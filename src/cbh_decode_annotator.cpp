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

#include "cbh_decode_annotator.h"

constexpr int ANNOTATOR_HEADER_FIXED_SIZE = 28; // without extra
constexpr int ANNOTATOR_ENTRY_SIZE = 62;

CbhAnnotatorDecoder::CbhAnnotatorDecoder(const char* filename, fileModeT fmode)
    : CbhDecoder(filename, fmode) {}

errorT CbhAnnotatorDecoder::decode_header() {
	if (auto err = stream_.open(filename_, fmode_))
		return err;

	stream_.pubseekpos(ANNOTATOR_HEADER_FIXED_SIZE - 4);

	char extra[1];
	stream_.sgetn(extra, 1);
	annotator_header_size_ = ANNOTATOR_HEADER_FIXED_SIZE +
	                         static_cast<byte>(extra[0]);

	return OK;
}

errorT CbhAnnotatorDecoder::decode_record(Game& game,
                                          std::vector<uint32_t> offsets) {
	uint32_t annotator = offsets.at(0);
	stream_.pubseekpos(annotator_header_size_ +
	                   annotator * ANNOTATOR_ENTRY_SIZE +
	                   9); // move to offset 9
	char name[46] = {0};
	stream_.sgetn(name, 45);

	if (*name) {
		game.addTag("Annotator", name);
	}

	return OK;
}
