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

#include "cbh.h"
#include "interface.h"
#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <random>

namespace {

const char* databases[] = {
    TESTDIR "NormalGames/BielMTO.cbh",
    TESTDIR "NonStandard/NonStandardStart.cbh",
    TESTDIR "WithVariations/WithVariations.cbh",
    TESTDIR "Chess960Biel/Chess960Biel.cbh",
    TESTDIR "NormalNoPop/StandardMissingPop.cbh",
    TESTDIR "ManyPromotions/ManyPromotions.cbh",
    TESTDIR "Chess960NonStandard/Chess960NonStandard.cbh",
};

const char* unusualBase = TESTDIR
    "UnusualStart/UnusualStartBytes.cbh"; // 9 games from the MegaBase 2021
                                          // which start with unusual bytes (1,
                                          // 4 or 5) and can be decoded by
                                          // Chessbase.
const char* corruptBase = TESTDIR
    "Chess960Corrupt/Corrupt.cbh"; // 4 from the MegaBase 2021 which can't be
                                   // decoded by Chessbase either (and make
                                   // Chessbase crash when trying to open in one
                                   // case)

// Database with guiding text
// Games with null moves

} // namespace

TEST(Test_codec, gameTest) {
	for (auto cbhname : databases) {

		CbhCodec codec;
		ASSERT_EQ(OK, codec.open(cbhname));

		auto numGames = codec.numGames();

		for (int i = 0; i < numGames; i++) {
			GameReturnValue game;
			codec.parseNext(game);
			EXPECT_GE(game.annotatedMoves.size(), 0);
			EXPECT_STRNE(game.startFen.c_str(), "");
		}
	}
}

TEST(Test_codec, doNotCrashWithUnusualStartByte) {
	CbhCodec codec;
	ASSERT_EQ(OK, codec.open(unusualBase));
	ASSERT_EQ(9, codec.numGames());

	for (int i = 0; i < codec.numGames(); i++) {
		GameReturnValue game;
		codec.parseNext(game);
	}
}

TEST(Test_codec, doNotCrashWithCorruptChess960Encoding) {
	CbhCodec codec;
	ASSERT_EQ(OK, codec.open(corruptBase));
	ASSERT_EQ(4, codec.numGames());

	for (int i = 0; i < codec.numGames(); i++) {
		GameReturnValue game;
		codec.parseNext(game);
	}
}
