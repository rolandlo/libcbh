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

/** @file
 * Abstract class for decoding files belonging to the CBH-format.
 */

#pragma once
#include "filebuf.h"
#include "interface.h"
#include <vector>

typedef unsigned short errorT;

class CbhDecoder {

public:
	FilebufAppend stream_;
	const char* filename_;

	CbhDecoder(const char* filename) : filename_(filename) {}

	virtual errorT open() { return stream_.open(filename_, FMODE_ReadOnly); };
	virtual errorT flush() {
		return (stream_.pubsync() == 0) ? OK : ERROR_FileWrite;
	}
	virtual errorT decode_header() = 0;
	virtual errorT decode_record(GameReturnValue& game,
	                             std::vector<uint32_t> offsets) = 0;
};