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
 * Implements a decoder for .cbg files.
 */

#pragma once
#include "cbh_decode_annotation.h"
#include "cbh_decode_base.h"
#include "cbh_position.h"
#include "error.h"
#include "interface.h"
#include <vector>

/**
 * This class implements a CBG game decoder that invokes the appropriate member
 * functions of the associated Game object for each type of CBG token.
 */
class CbhGameDecoder final : public CbhDecoder {

public:
	CbhGameDecoder(const char* gameFilename, const char* annotationFilename);

	errorT open() override;
	errorT flush() override;

	errorT decode_header() override;
	errorT decode_record(GameReturnValue& game,
	                     std::vector<uint32_t> offsets) override;

private:
	enum { Token_Move, Token_Push, Token_Pop, Token_Skip };

	CbhAnnotationDecoder annotationDecoder;
	decoder::PositionStack position_;
	bool is_chess960;
	const byte* lookup;
	uint32_t bytes_read_;  // number of bytes read from the current game
	uint32_t bytes_total_; // number of bytes in total for the game

	errorT startDecoding(std::string& startFen);

	uint32_t decodeMoves(std::vector<AnnotatedMove>& moves,
	                     uint32_t move_number = 0);
	byte translate_byte(byte b, int count);

	uint32_t decodeMove(simpleMoveT& sm, byte move_code, uint32_t move_number); 
};
