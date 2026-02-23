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

#include "codec_cbh.h"

/* -----------------------------------------------------------------------------
See
https://talkchess.com/viewtopic.php?p=287896&sid=7237cb8fc0656ad4837fc9ab29cf802f#p287896
for a reference on the Chessbase .cbh database format
*/

constexpr int INDEX_HEADER_SIZE = 46;
constexpr int INDEX_ENTRY_SIZE = 46;


errorT CodecCBH::open(const char* filename) {
	ASSERT(filename);
	const fileModeT fmode = FMODE_ReadOnly;

	auto dbname = std::string_view(filename);
	if (dbname.ends_with(".cbh"))
		dbname.remove_suffix(4);

	if (dbname.empty())
		return ERROR_FileOpen;

	filenames_.resize(7);
	filenames_[0].assign(dbname).append(".cbh"); // header
	filenames_[1].assign(dbname).append(".cbp"); // player data
	filenames_[2].assign(dbname).append(".cbt"); // tournament data
	filenames_[3].assign(dbname).append(".cbc"); // annotator data
	filenames_[4].assign(dbname).append(".cbs"); // source data
	filenames_[5].assign(dbname).append(".cbg"); // game data
	filenames_[6].assign(dbname).append(".cba"); // annotation data

	player_decoder = std::make_unique<CbhPlayerDecoder>(filenames_[1].c_str(),
	                                                    fmode);
	tournament_decoder = std::make_unique<CbhTournamentDecoder>(
	    filenames_[2].c_str(), fmode);
	annotator_decoder = std::make_unique<CbhAnnotatorDecoder>(
	    filenames_[3].c_str(), fmode);
	source_decoder = std::make_unique<CbhSourceDecoder>(filenames_[4].c_str(),
	                                                    fmode);
	game_decoder = std::make_unique<CbhGameDecoder>(
	    filenames_[5].c_str(), filenames_[6].c_str(), fmode);

	if (fmode == FMODE_Create) {
		for (auto const& fname : filenames_) {
			std::error_code ec;
			if (std::filesystem::exists(fname, ec) || ec)
				return ERROR_Exists;
		}

		if (auto err = idxfile_.Open(filenames_[0].c_str(), fmode))
			return err;

		if (auto err = player_decoder->open())
			return err;

		if (auto err = tournament_decoder->open())
			return err;

		if (auto err = annotator_decoder->open())
			return err;

		if (auto err = source_decoder->open())
			return err;

		if (auto err = game_decoder->open())
			return err;

		return OK;
	}

	auto err_idx = read_index_header(fmode, filenames_[0].c_str());
	auto err_pl = player_decoder->decode_header();
	auto err_to = tournament_decoder->decode_header();
	auto err_ar = annotator_decoder->decode_header();
	auto err_so = source_decoder->decode_header();
	auto err_gm = game_decoder->decode_header();

	return err_idx  ? err_idx
	       : err_pl ? err_pl
	       : err_to ? err_to
	       : err_ar ? err_ar
	       : err_so ? err_so
	                : err_gm;
}

errorT CodecCBH::parseNext(Game& game) {
	if (n_parsed_ >= n_games_)
		return ERROR_NotFound;

	byte flags = idxfile_.ReadOneByte();
	bool guiding_text = (flags & 0x2);
	if (guiding_text) { // TODO, skip for now
		idxfile_.pubseekoff(INDEX_ENTRY_SIZE - 1, std::ios::cur, std::ios::in);
		n_parsed_ += 1;
		return ERROR_Decode;
	}

	uint32_t game_offset = idxfile_.ReadFourBytes();
	uint32_t annotation_offset = idxfile_.ReadFourBytes();
	uint white_player = idxfile_.ReadThreeBytes();
	uint black_player = idxfile_.ReadThreeBytes();
	uint tournament = idxfile_.ReadThreeBytes();
	uint annotator = idxfile_.ReadThreeBytes();
	uint source = idxfile_.ReadThreeBytes();
	uint date = idxfile_.ReadThreeBytes();
	byte res = idxfile_.ReadOneByte();
	byte line_eval = idxfile_.ReadOneByte();
	byte round = idxfile_.ReadOneByte();
	byte subround = idxfile_.ReadOneByte();
	uint16_t white_rating = idxfile_.ReadTwoBytes();
	uint16_t black_rating = idxfile_.ReadTwoBytes();
	uint16_t eco = idxfile_.ReadTwoBytes();
	idxfile_.pubseekoff(INDEX_ENTRY_SIZE - 37, std::ios::cur, std::ios::in);

	uint year = (date >> 9) & 4095;
	uint month = (date >> 5) & 15;
	uint day = date & 31;

	std::string round_string = std::to_string(round) + "." +
	                           std::to_string(subround);

	resultT result = res == 2   ? RESULT_White
	                 : res == 1 ? RESULT_Draw
	                 : res == 0 ? RESULT_Black
	                            : RESULT_None;

	// bits 7-15 for for eco; 0->0, 1->A00 = 132, 2-> A01 = 263,...
	// ignore subcodes (bits 0-6)
	ecoT scidEco = (eco >> 7) ? ((eco >> 7) - 1) * 131 + 1 : 0;

	game.Clear();
	game.SetDate(DATE_MAKE(year, month, day));
	game.SetWhiteElo(white_rating & 0xFFF);
	game.SetBlackElo(black_rating & 0xFFF);
	game.SetEco(scidEco);
	game.SetRoundStr(round_string.c_str());
	game.SetResult(result);

	errorT err_player = player_decoder->decode_record(
	    game, std::vector<uint32_t>{white_player, black_player});
	errorT err_tournament = tournament_decoder->decode_record(
	    game, std::vector<uint32_t>{tournament});
	errorT err_annotator = annotator_decoder->decode_record(
	    game, std::vector<uint32_t>{annotator});
	errorT err_source = source_decoder->decode_record(
	    game, std::vector<uint32_t>{source});
	errorT err_game = game_decoder->decode_record(
	    game, std::vector<uint32_t>{game_offset, annotation_offset});

	n_parsed_ += 1;

	return err_player       ? err_player
	       : err_tournament ? err_tournament
	       : err_annotator  ? err_annotator
	       : err_source     ? err_source
	                        : err_game;
}

std::pair<size_t, size_t> CodecCBH::parseProgress() {
	return std::make_pair(n_parsed_, n_games_);
}

size_t CodecCBH::numGames() { return n_games_; }

const char* CodecCBH::parseErrors() { return NULL; }

errorT CodecCBH::read_index_header(fileModeT fmode, const char* fname) {
	if (auto err = idxfile_.Open(fname, fmode))
		return err;

	const auto file_size = idxfile_.pubseekoff(0, std::ios::end);
	int entries_size = static_cast<int>(file_size) - INDEX_HEADER_SIZE;
	if (entries_size < 0 || (entries_size % INDEX_ENTRY_SIZE) != 0 ||
	    idxfile_.pubseekoff(0, std::ios::beg) != 0)
		return ERROR_Corrupt;

	n_games_ = entries_size / INDEX_ENTRY_SIZE;

	uint head1 = idxfile_.ReadThreeBytes();
	uint head2 = idxfile_.ReadThreeBytes();
	if ((head1 != 0x00002C && head1 != 0x000024) || head2 != 0x002E01)
		return ERROR_BadMagic;

	const std::streamsize remaining = INDEX_HEADER_SIZE - 6;
	char dummy[remaining];
	idxfile_.sgetn(dummy, remaining);

	return OK;
}
