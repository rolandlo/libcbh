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
 * Public header for the CbhCodec class.
 */

#pragma once

#include <memory>
#include "error.h"
#include "interface.h"

// Macro to control visibility
#ifdef _WIN32
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif

class CbhCodecImpl;

// This class manages databases encoded in Chessbase's cbh format.
class CbhCodec final {

public:
	CbhCodec() EXPORT_SYMBOL;
	~CbhCodec() EXPORT_SYMBOL;

	/**
	 * Opens a CBH database.
	 * After successfully opening the file, the object is ready for
	 * parseNext() calls.
	 * @param filename: full path of the cbh file to be opened.
	 * @param fmode:    valid file access mode.
	 * @returns OK in case of success, an @e errorT code otherwise.
	 */
	errorT open(const char* filename) EXPORT_SYMBOL;

	/**
	 * Reads the next game.
	 * @param game: the Game object where the data will be stored.
	 * @returns
	 * - ERROR_NotFound if there are no more games to be read.
	 * - OK otherwise.
	 */
	errorT parseNext(GameReturnValue& game) EXPORT_SYMBOL;

	/**
	 * Sets the index of the game to be parsed next.
	 * This needs to be used if not all games from index 0
	 * on are to be parsed.
	 * @param index: the index of the game
	 * @returns
	 * - ERROR_NOTFound if index >= numGames()
	 * - OK otherwise.
	 */
	errorT setGameIndex(uint32_t index) EXPORT_SYMBOL;

	/**
	 * The number of games in the database parsed
	 */
	size_t numParsed() EXPORT_SYMBOL;

	/**
	 * The number of games in the database according to the index header
	 */
	size_t numGames() EXPORT_SYMBOL;

	/**
	 * Return the list of files that are decoded
	 */
	std::vector<std::string> getFilenames() const EXPORT_SYMBOL;

	/**
	 * Returns the list of errors produced by parseNext() calls.
	 */
	const char* parseErrors() EXPORT_SYMBOL;

private:
	std::unique_ptr<CbhCodecImpl> pImpl; // Pointer to implementation
};
