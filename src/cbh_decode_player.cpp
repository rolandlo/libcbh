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

#include "cbh_decode_player.h"

constexpr int PLAYER_HEADER_FIXED_SIZE = 28; // without extra
constexpr int PLAYER_ENTRY_SIZE = 67;

CbhPlayerDecoder::CbhPlayerDecoder(const char* filename, fileModeT fmode)
    : CbhDecoder(filename, fmode) {}

errorT CbhPlayerDecoder::decode_header() {
	if (auto err = stream_.open(filename_, fmode_))
		return err;

	stream_.pubseekpos(PLAYER_HEADER_FIXED_SIZE - 4);

	char extra[1];
	stream_.sgetn(extra, 1);
	player_header_size_ = PLAYER_HEADER_FIXED_SIZE +
	                      static_cast<byte>(extra[0]);

	return OK;
}

errorT CbhPlayerDecoder::decode_record(Game& game,
                                       std::vector<uint32_t> offsets) {
	uint32_t white_player = offsets.at(0);
	uint32_t black_player = offsets.at(1);

	auto player_string = [&](uint32_t player_offset) {
		stream_.pubseekpos(player_header_size_ +
		                   player_offset * PLAYER_ENTRY_SIZE +
		                   9); // move to offset 9
		char last_name[31] = {0};
		char first_name[21] = {0};
		stream_.sgetn(last_name, 30);
		stream_.sgetn(first_name, 20);
		// printf("%s, %s\n", last_name, first_name);
		return std::string(last_name) + ", " + std::string(first_name);
	};

	game.SetWhiteStr(player_string(white_player).c_str());
	game.SetBlackStr(player_string(black_player).c_str());

	return OK;
}
