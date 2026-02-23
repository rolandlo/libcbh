/*
 * Copyright (C) 2026 Roland Lötscher
 *
 * Scid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * Scid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scid. If not, see <http://www.gnu.org/licenses/>.
 */

#include "codec_cbh.h"
#include "game.h"
#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <random>

namespace {

const char* databases[] = {
    SCID_TESTDIR "NormalGames/BielMTO.cbh",
    SCID_TESTDIR "NonStandard/NonStandardStart.cbh",
    SCID_TESTDIR "WithVariations/WithVariations.cbh",
    SCID_TESTDIR "Chess960Biel/Chess960Biel.cbh",
    SCID_TESTDIR "NormalNoPop/StandardMissingPop.cbh",
    SCID_TESTDIR "ManyPromotions/ManyPromotions.cbh",
    SCID_TESTDIR "Chess960NonStandard/Chess960NonStandard.cbh",
};

const char* unusualBase = SCID_TESTDIR
    "UnusualStart/UnusualStartBytes.cbh"; // 9 games from the MegaBase 2021
                                          // which start with unusual bytes (1,
                                          // 4 or 5) and can be decoded by
                                          // Chessbase.
const char* corruptBase = SCID_TESTDIR
    "Chess960Corrupt/Corrupt.cbh"; // 4 from the MegaBase 2021 which can't be
                                   // decoded by Chessbase either (and make
                                   // Chessbase crash when trying to open in one
                                   // case)

// Database with guiding text
// Games with null moves

} // namespace

TEST(Test_codec, gameTest) {
	for (auto cbhname : databases) {

		CodecCBH codec;
		ASSERT_EQ(OK, codec.open(cbhname));

		auto numGames = codec.numGames();

		for (int i = 0; i < numGames; i++) {
			Game game;
			codec.parseNext(game);
			EXPECT_GE(game.GetNumHalfMoves(), 0);
			EXPECT_GE(game.GetNumVariations(), 0);

			game.MoveToStart();
			char start[1024];
			game.currentPos()->PrintFEN(start);
			EXPECT_STRNE(start, "");

			game.MoveToEnd();
			char end[1024];
			game.currentPos()->PrintFEN(end);
			EXPECT_STRNE(end, "");
			EXPECT_STRNE(game.currentPosUCI().c_str(), "");
		}
	}
}

TEST(Test_codec, doNotCrashWithUnusualStartByte) {
	CodecCBH codec;
	ASSERT_EQ(OK, codec.open(unusualBase));
	ASSERT_EQ(9, codec.numGames());

	for (int i = 0; i < codec.numGames(); i++) {
		Game game;
		codec.parseNext(game);
	}
}

TEST(Test_codec, doNotCrashWithCorruptChess960Encoding) {
	CodecCBH codec;
	ASSERT_EQ(OK, codec.open(corruptBase));
	ASSERT_EQ(4, codec.numGames());

	for (int i = 0; i < codec.numGames(); i++) {
		Game game;
		codec.parseNext(game);
	}
}
