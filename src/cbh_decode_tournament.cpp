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

#include "cbh_decode_tournament.h"

constexpr int TOURNAMENT_HEADER_FIXED_SIZE = 28; // without extra
constexpr int TOURNAMENT_ENTRY_SIZE = 99;

CbhTournamentDecoder::CbhTournamentDecoder(const char* filename,
                                           fileModeT fmode)
    : CbhDecoder(filename, fmode) {}

errorT CbhTournamentDecoder::decode_header() {
	if (auto err = stream_.open(filename_, fmode_))
		return err;

	stream_.pubseekpos(TOURNAMENT_HEADER_FIXED_SIZE - 4);

	char extra[1];
	stream_.sgetn(extra, 1);
	tournament_header_size_ = TOURNAMENT_HEADER_FIXED_SIZE +
	                          static_cast<byte>(extra[0]);

	return OK;
}

errorT CbhTournamentDecoder::decode_record(Game& game,
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

	game.SetEventStr(title);
	game.SetSiteStr(place);
	game.SetEventDate(DATE_MAKE(year, month, day));

	return OK;
}
