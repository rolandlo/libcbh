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

#include "cbh_decode_player.h"

constexpr int PLAYER_HEADER_FIXED_SIZE = 28; // without extra
constexpr int PLAYER_ENTRY_SIZE = 67;

CbhPlayerDecoder::CbhPlayerDecoder(const char* filename)
    : CbhDecoder(filename) {}

errorT CbhPlayerDecoder::decode_header() {
	if (auto err = stream_.open(filename_, FMODE_ReadOnly))
		return err;

	stream_.pubseekpos(PLAYER_HEADER_FIXED_SIZE - 4);

	char extra[1];
	stream_.sgetn(extra, 1);
	player_header_size_ = PLAYER_HEADER_FIXED_SIZE +
	                      static_cast<byte>(extra[0]);

	return OK;
}

errorT CbhPlayerDecoder::decode_record(GameReturnValue& game,
                                       std::vector<uint32_t> offsets) {
	uint32_t white_player = offsets.at(0);
	uint32_t black_player = offsets.at(1);

	auto set_player_string = [&](uint32_t player_offset, std::string& last,
	                             std::string& first) {
		stream_.pubseekpos(player_header_size_ +
		                   player_offset * PLAYER_ENTRY_SIZE +
		                   9); // move to offset 9
		char last_name[31] = {0};
		char first_name[21] = {0};
		stream_.sgetn(last_name, 30);
		stream_.sgetn(first_name, 20);
		last = last_name;
		first = first_name;
	};

	set_player_string(white_player, game.whiteName, game.whiteFirstName);
	set_player_string(black_player, game.blackName, game.blackFirstName);

	return OK;
}
