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

#include "board_def.h"
#include "cbh.h"
#include "interface.h"
#include "nags.h"
#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <random>
#include <variant>
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

const char* annotationBase = TESTDIR "Annotation/TestBase.cbh";

TEST(Test_codec, AnnotationTest) {
	CbhCodec codec;
	ASSERT_EQ(OK, codec.open(annotationBase));
	ASSERT_EQ(4, codec.numGames());

	GameReturnValue game;
	codec.parseNext(game);
	auto moves = game.annotatedMoves;
	ASSERT_EQ(10, moves.size());

	std::tuple<byte, byte, byte> symbolData[8]{
	    {D2, D4, NAG_GoodMove},        {D7, D5, NAG_PoorMove},
	    {C2, C4, NAG_InterestingMove}, {C7, C6, NAG_DubiousMove},
	    {G1, F3, NAG_ExcellentMove},   {G8, F6, NAG_Blunder},
	    {B1, C3, NAG_WhiteZugZwang},   {E7, E6, NAG_OnlyMove},
	};
	int num = 0;
	for (const auto& [from, to, symbol] : symbolData) {
		ASSERT_EQ(from, moves[num].from);
		ASSERT_EQ(to, moves[num].to);
		ASSERT_EQ(1u, moves[num].comments.size());
		auto comment = moves[num].comments[0];
		ASSERT_EQ(true, std::holds_alternative<SymbolComment>(comment));
		ASSERT_EQ(symbol, std::get<SymbolComment>(comment).symbol);
		ASSERT_EQ(0u, std::get<SymbolComment>(comment).evaluation);
		ASSERT_EQ(0u, std::get<SymbolComment>(comment).prefix);
		num++;
	}
	ASSERT_EQ(moves[8].from, C1);
	ASSERT_EQ(moves[8].to, G5);
	ASSERT_EQ(true, moves[8].comments.empty());
	ASSERT_EQ(byte(-2), moves[9].promote); // pop

	game.annotatedMoves.clear();
	codec.parseNext(game);
	moves = game.annotatedMoves;
	ASSERT_EQ(17, moves.size());
	std::tuple<byte, byte, byte> evaluationData[15]{
	    {E2, E4, NAG_WhiteDecisive},
	    {E7, E5, NAG_WhiteClear},
	    {G1, F3, NAG_WhiteSlight},
	    {B8, C6, NAG_Equal},
	    {F1, B5, NAG_Unclear},
	    {G8, F6, NAG_BlackSlight},
	    {E1, G1, NAG_BlackClear},
	    {F6, E4, NAG_BlackDecisive},
	    {D2, D4, NAG_WhiteCompensation},
	    {E4, D6, NAG_BlackWithAttack},
	    {B5, C6, NAG_WhiteWithInitiative},
	    {D7, C6, NAG_BlackCounterPlay},
	    {D4, E5, NAG_WhiteTimeControlPressure},
	    {D6, F5, NAG_BlackModerateDevelopmentAdvantage},
	    {D1, D8, NAG_Novelty},
	};
	num = 0;
	for (const auto& [from, to, evaluation] : evaluationData) {
		ASSERT_EQ(from, moves[num].from);
		ASSERT_EQ(to, moves[num].to);
		ASSERT_EQ(1u, moves[num].comments.size());
		auto comment = moves[num].comments[0];
		ASSERT_EQ(true, std::holds_alternative<SymbolComment>(comment));
		ASSERT_EQ(0u, std::get<SymbolComment>(comment).symbol);
		ASSERT_EQ(evaluation, std::get<SymbolComment>(comment).evaluation);
		ASSERT_EQ(0u, std::get<SymbolComment>(comment).prefix);
		num++;
	}
	ASSERT_EQ(moves[15].from, E8);
	ASSERT_EQ(moves[15].to, D8);
	ASSERT_EQ(true, moves[15].comments.empty());
	ASSERT_EQ(byte(-2), moves[16].promote); // pop

	game.annotatedMoves.clear();
	codec.parseNext(game);
	moves = game.annotatedMoves;
	ASSERT_EQ(8, moves.size());
	std::tuple<byte, byte, byte> prefixData[6]{
	    {D2, D4, NAG_EditorialComment}, {G8, F6, NAG_BetterIs},
	    {C2, C4, NAG_WorseIs},          {E7, E6, NAG_EquivalentIs},
	    {B1, C3, NAG_WithIdea},         {F8, B4, NAG_AimedAgainst},
	};
	num = 0;
	for (const auto& [from, to, prefix] : prefixData) {
		ASSERT_EQ(from, moves[num].from);
		ASSERT_EQ(to, moves[num].to);
		ASSERT_EQ(1u, moves[num].comments.size());
		auto comment = moves[num].comments[0];
		ASSERT_EQ(true, std::holds_alternative<SymbolComment>(comment));
		ASSERT_EQ(0u, std::get<SymbolComment>(comment).symbol);
		ASSERT_EQ(0u, std::get<SymbolComment>(comment).evaluation);
		ASSERT_EQ(prefix, std::get<SymbolComment>(comment).prefix);
		num++;
	}
	ASSERT_EQ(moves[6].from, E2);
	ASSERT_EQ(moves[6].to, E3);
	ASSERT_EQ(true, moves[6].comments.empty());
	ASSERT_EQ(byte(-2), moves[7].promote); // pop

	game.annotatedMoves.clear();
	codec.parseNext(game);
	moves = game.annotatedMoves;
	ASSERT_EQ(6, moves.size());

	ASSERT_EQ(C2, moves[0].from);
	ASSERT_EQ(C4, moves[0].to);
	ASSERT_EQ(1u, moves[0].comments.size());
	auto comment = moves[0].comments[0];
	ASSERT_EQ(true, std::holds_alternative<TextBeforeComment>(comment));
	auto textBefore = std::get<TextBeforeComment>(comment);
	ASSERT_EQ(0u, textBefore.lang);
	ASSERT_STREQ("This is a text before move comment", textBefore.text.c_str());

	ASSERT_EQ(E7, moves[1].from);
	ASSERT_EQ(E5, moves[1].to);
	ASSERT_EQ(1u, moves[1].comments.size());
	auto comment1 = moves[1].comments[0];
	ASSERT_EQ(true, std::holds_alternative<TextAfterComment>(comment1));
	auto textAfter1 = std::get<TextAfterComment>(comment1);
	ASSERT_EQ(0u, textAfter1.lang);
	ASSERT_STREQ("This is a text after move comment", textAfter1.text.c_str());

	ASSERT_EQ(B1, moves[2].from);
	ASSERT_EQ(C3, moves[2].to);
	ASSERT_EQ(1u, moves[2].comments.size());
	auto comment2 = moves[2].comments[0];
	ASSERT_EQ(true, std::holds_alternative<TextAfterComment>(comment2));
	auto textAfter2 = std::get<TextAfterComment>(comment2);
	ASSERT_EQ(0x35, textAfter2.lang);
	ASSERT_STREQ("Dieser Text nach Zug Kommentar ist auf Deutsch",
	             textAfter2.text.c_str());

	ASSERT_EQ(moves[3].from, G8);
	ASSERT_EQ(moves[3].to, F6);
	ASSERT_EQ(true, moves[3].comments.empty());
	ASSERT_EQ(moves[4].from, G1);
	ASSERT_EQ(moves[4].to, F3);
	ASSERT_EQ(true, moves[4].comments.empty());
	ASSERT_EQ(byte(-2), moves[5].promote); // pop
}