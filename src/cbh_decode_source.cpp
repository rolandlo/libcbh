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

#include "cbh_decode_source.h"
#include <sstream>

constexpr int SOURCE_HEADER_FIXED_SIZE = 28; // without extra
constexpr int SOURCE_ENTRY_SIZE = 68;

CbhSourceDecoder::CbhSourceDecoder(const char* filename)
    : CbhDecoder(filename) {}

errorT CbhSourceDecoder::decode_header() {
	if (auto err = stream_.open(filename_, FMODE_ReadOnly))
		return err;

	stream_.pubseekpos(SOURCE_HEADER_FIXED_SIZE - 4);

	char extra[1];
	stream_.sgetn(extra, 1);
	source_header_size_ = SOURCE_HEADER_FIXED_SIZE +
	                      static_cast<byte>(extra[0]);

	return OK;
}

errorT CbhSourceDecoder::decode_record(GameReturnValue& game,
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
		game.tags.emplace_back("SourceTitle", title);
	if (*publisher)
		game.tags.emplace_back("Source", publisher);
	if (sourceDate != "0.0.0")
		game.tags.emplace_back("SourceDate", sourceDate);
	if (sourceVersionDate != "0.0.0")
		game.tags.emplace_back("SourceVersionDate", sourceVersionDate);
	if (sourceVersion != "0")
		game.tags.emplace_back("SourceVersion", sourceVersion);
	if (sourceQuality != "0")
		game.tags.emplace_back("SourceQuality", sourceQuality);

	return OK;
}
