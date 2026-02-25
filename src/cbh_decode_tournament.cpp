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

#include "cbh_decode_tournament.h"

constexpr int TOURNAMENT_HEADER_FIXED_SIZE = 28; // without extra
constexpr int TOURNAMENT_ENTRY_SIZE = 99;

CbhTournamentDecoder::CbhTournamentDecoder(const char* filename)
    : CbhDecoder(filename) {}

errorT CbhTournamentDecoder::decode_header() {
	if (auto err = stream_.open(filename_, FMODE_ReadOnly))
		return err;

	stream_.pubseekpos(TOURNAMENT_HEADER_FIXED_SIZE - 4);

	char extra[1];
	stream_.sgetn(extra, 1);
	tournament_header_size_ = TOURNAMENT_HEADER_FIXED_SIZE +
	                          static_cast<byte>(extra[0]);

	return OK;
}

errorT CbhTournamentDecoder::decode_record(GameReturnValue& game,
                                           std::vector<uint32_t> offsets) {
	uint32_t tournament = offsets.at(0);
	stream_.pubseekpos(tournament_header_size_ +
	                   tournament * TOURNAMENT_ENTRY_SIZE +
	                   9); // move to offset 9
	char title[41] = {0};
	char place[31] = {0};
	char dateStr[3];
	stream_.sgetn(title, 40);
	stream_.sgetn(place, 30);
	stream_.sgetn(dateStr, 3);
	uint32_t date = static_cast<byte>(dateStr[2]) << 16 |
	                static_cast<byte>(dateStr[1]) << 8 |
	                static_cast<byte>(dateStr[0]); // little endian
	uint year = (date >> 9) & 4095;
	uint month = (date >> 5) & 15;
	uint day = date & 31;

	game.event = title;
	game.eventPlace = place;
	game.eventDate = Date(year, month, day);

	return OK;
}
