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

#include "cbh_decode_annotator.h"

constexpr int ANNOTATOR_HEADER_FIXED_SIZE = 28; // without extra
constexpr int ANNOTATOR_ENTRY_SIZE = 62;

CbhAnnotatorDecoder::CbhAnnotatorDecoder(const char* filename)
    : CbhDecoder(filename) {}

errorT CbhAnnotatorDecoder::decode_header() {
	if (auto err = stream_.open(filename_, FMODE_ReadOnly))
		return err;

	stream_.pubseekpos(ANNOTATOR_HEADER_FIXED_SIZE - 4);

	char extra[1];
	stream_.sgetn(extra, 1);
	annotator_header_size_ = ANNOTATOR_HEADER_FIXED_SIZE +
	                         static_cast<byte>(extra[0]);

	return OK;
}

errorT CbhAnnotatorDecoder::decode_record(GameReturnValue& game,
                                          std::vector<uint32_t> offsets) {
	uint32_t annotator = offsets.at(0);
	stream_.pubseekpos(annotator_header_size_ +
	                   annotator * ANNOTATOR_ENTRY_SIZE +
	                   9); // move to offset 9
	char name[46] = {0};
	stream_.sgetn(name, 45);

	if (*name) {
		game.tags.emplace_back("Annotator", name);
	}

	return OK;
}
