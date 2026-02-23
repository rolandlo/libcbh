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

#include "cbh_decode_source.h"
#include <sstream>

constexpr int SOURCE_HEADER_FIXED_SIZE = 28; // without extra
constexpr int SOURCE_ENTRY_SIZE = 68;

CbhSourceDecoder::CbhSourceDecoder(const char* filename, fileModeT fmode)
    : CbhDecoder(filename, fmode) {}

errorT CbhSourceDecoder::decode_header() {
	if (auto err = stream_.open(filename_, fmode_))
		return err;

	stream_.pubseekpos(SOURCE_HEADER_FIXED_SIZE - 4);

	char extra[1];
	stream_.sgetn(extra, 1);
	source_header_size_ = SOURCE_HEADER_FIXED_SIZE +
	                      static_cast<byte>(extra[0]);

	return OK;
}

errorT CbhSourceDecoder::decode_record(Game& game,
                                       std::vector<uint32_t> offsets) {
	uint32_t source = offsets.at(0);
	stream_.pubseekpos(source_header_size_ + source * SOURCE_ENTRY_SIZE +
	                   9); // move to offset 9
	char title[26] = {0};
	stream_.sgetn(title, 25);

	char publisher[17] = {0};
	stream_.sgetn(publisher, 16);

	auto getDateStr = [this]() {
		char dateStr[3];
		stream_.sgetn(dateStr, 3);
		uint32_t date = static_cast<byte>(dateStr[2]) << 16 |
		                static_cast<byte>(dateStr[1]) << 8 |
		                static_cast<byte>(dateStr[0]); // little endian
		uint year = (date >> 9) & 4095;
		uint month = (date >> 5) & 15;
		uint day = date & 31;
		std::ostringstream ret;
		ret << year << "." << month << "." << day;
		return ret.str();
	};

	std::string sourceDate = getDateStr();
	stream_.sbumpc(); // skip
	std::string sourceVersionDate = getDateStr();
	stream_.sbumpc(); // skip
	std::string sourceVersion = std::to_string(stream_.sbumpc());
	std::string sourceQuality = std::to_string(stream_.sbumpc());

	if (*title)
		game.addTag("SourceTitle", title);
	if (*publisher)
		game.addTag("Source", publisher);
	if (sourceDate != "0.0.0")
		game.addTag("SourceDate", sourceDate);
	if (sourceVersionDate != "0.0.0")
		game.addTag("SourceVersionDate", sourceVersionDate);
	if (sourceVersion != "0")
		game.addTag("SourceVersion", sourceVersion);
	if (sourceQuality != "0")
		game.addTag("SourceQuality", sourceQuality);

	return OK;
}
