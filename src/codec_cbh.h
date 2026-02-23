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
 * Implements the CodecCBH class.
 */

#pragma once

#include "cbh_decode_annotator.h"
#include "cbh_decode_game.h"
#include "cbh_decode_player.h"
#include "cbh_decode_source.h"
#include "cbh_decode_tournament.h"
#include "filebuf.h"
#include <filesystem>
#include <vector>

// This class manages databases encoded in Chessbase's cbh format.
class CodecCBH final {
	Filebuf idxfile_; // header file

	std::vector<std::string> filenames_;

	size_t n_games_ = 0;
	size_t n_parsed_ = 0;

	std::unique_ptr<CbhDecoder> player_decoder;
	std::unique_ptr<CbhDecoder> tournament_decoder;
	std::unique_ptr<CbhDecoder> annotator_decoder;
	std::unique_ptr<CbhDecoder> source_decoder;
	std::unique_ptr<CbhDecoder> game_decoder;

public:
	/**
	 * Opens a CBH database.
	 * After successfully opening the file, the object is ready for
	 * parseNext() calls.
	 * @param filename: full path of the cbh file to be opened.
	 * @param fmode:    valid file access mode.
	 * @returns OK in case of success, an @e errorT code otherwise.
	 */
	errorT open(const char* filename);

	/**
	 * Reads the next game.
	 * @param game: the Game object where the data will be stored.
	 * @returns
	 * - ERROR_NotFound if there are no more games to be read.
	 * - OK otherwise.
	 */
	errorT parseNext(Game& game);

	/**
	 * Returns info about the parsing progress.
	 * @returns a pair<size_t, size_t> where first element is the quantity of
	 * data parsed and second one is the total amount of data of the database.
	 */
	std::pair<size_t, size_t> parseProgress();

	/**
	 * Returns the list of errors produced by parseNext() calls.
	 */
	const char* parseErrors();

private:
	errorT read_index_header(fileModeT fmode, const char* fname);
};
