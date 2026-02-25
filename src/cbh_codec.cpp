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

#include "cbh_codec.h"
#include "cbh.h"
#include "cbh_decode_annotator.h"
#include "cbh_decode_game.h"
#include "cbh_decode_player.h"
#include "cbh_decode_source.h"
#include "cbh_decode_tournament.h"
#include "interface.h"

/* -----------------------------------------------------------------------------
See
https://talkchess.com/viewtopic.php?p=287896&sid=7237cb8fc0656ad4837fc9ab29cf802f#p287896
for a reference on the Chessbase .cbh database format
*/

constexpr int INDEX_HEADER_SIZE = 46;
constexpr int INDEX_ENTRY_SIZE = 46;

CbhCodec::CbhCodec() : pImpl(std::make_unique<CbhCodecImpl>()) {};
CbhCodec::~CbhCodec() = default;

errorT CbhCodec::open(const char* filename) {
	ASSERT(filename);

	auto dbname = std::string_view(filename);
	if (dbname.ends_with(".cbh"))
		dbname.remove_suffix(4);

	if (dbname.empty())
		return ERROR_FileOpen;

	pImpl->filenames_.resize(7);
	pImpl->filenames_[0].assign(dbname).append(".cbh"); // header
	pImpl->filenames_[1].assign(dbname).append(".cbp"); // player data
	pImpl->filenames_[2].assign(dbname).append(".cbt"); // tournament data
	pImpl->filenames_[3].assign(dbname).append(".cbc"); // annotator data
	pImpl->filenames_[4].assign(dbname).append(".cbs"); // source data
	pImpl->filenames_[5].assign(dbname).append(".cbg"); // game data
	pImpl->filenames_[6].assign(dbname).append(".cba"); // annotation data

	pImpl->player_decoder = std::make_unique<CbhPlayerDecoder>(
	    pImpl->filenames_[1].c_str());
	pImpl->tournament_decoder = std::make_unique<CbhTournamentDecoder>(
	    pImpl->filenames_[2].c_str());
	pImpl->annotator_decoder = std::make_unique<CbhAnnotatorDecoder>(
	    pImpl->filenames_[3].c_str());
	pImpl->source_decoder = std::make_unique<CbhSourceDecoder>(
	    pImpl->filenames_[4].c_str());
	pImpl->game_decoder = std::make_unique<CbhGameDecoder>(
	    pImpl->filenames_[5].c_str(), pImpl->filenames_[6].c_str());

	auto err_idx = pImpl->read_index_header(pImpl->filenames_[0].c_str());
	auto err_pl = pImpl->player_decoder->decode_header();
	auto err_to = pImpl->tournament_decoder->decode_header();
	auto err_ar = pImpl->annotator_decoder->decode_header();
	auto err_so = pImpl->source_decoder->decode_header();
	auto err_gm = pImpl->game_decoder->decode_header();

	return err_idx  ? err_idx
	       : err_pl ? err_pl
	       : err_to ? err_to
	       : err_ar ? err_ar
	       : err_so ? err_so
	                : err_gm;
}

errorT CbhCodec::parseNext(GameReturnValue& game) {
	if (pImpl->n_parsed_ >= pImpl->n_games_)
		return ERROR_NotFound;

	byte flags = pImpl->idxfile_.ReadOneByte();
	bool guiding_text = (flags & 0x2);
	if (guiding_text) { // TODO, skip for now
		pImpl->idxfile_.pubseekoff(INDEX_ENTRY_SIZE - 1, std::ios::cur,
		                           std::ios::in);
		pImpl->n_parsed_ += 1;
		return ERROR_Decode;
	}

	uint32_t game_offset = pImpl->idxfile_.ReadFourBytes();
	uint32_t annotation_offset = pImpl->idxfile_.ReadFourBytes();
	uint white_player = pImpl->idxfile_.ReadThreeBytes();
	uint black_player = pImpl->idxfile_.ReadThreeBytes();
	uint tournament = pImpl->idxfile_.ReadThreeBytes();
	uint annotator = pImpl->idxfile_.ReadThreeBytes();
	uint source = pImpl->idxfile_.ReadThreeBytes();
	uint date = pImpl->idxfile_.ReadThreeBytes();
	byte res = pImpl->idxfile_.ReadOneByte();
	byte line_eval = pImpl->idxfile_.ReadOneByte();
	byte round = pImpl->idxfile_.ReadOneByte();
	byte subround = pImpl->idxfile_.ReadOneByte();
	uint16_t white_rating = pImpl->idxfile_.ReadTwoBytes();
	uint16_t black_rating = pImpl->idxfile_.ReadTwoBytes();
	uint16_t eco = pImpl->idxfile_.ReadTwoBytes();
	pImpl->idxfile_.pubseekoff(INDEX_ENTRY_SIZE - 37, std::ios::cur,
	                           std::ios::in);

	uint year = (date >> 9) & 4095;
	uint month = (date >> 5) & 15;
	uint day = date & 31;

	resultT result = res == 2   ? RESULT_White
	                 : res == 1 ? RESULT_Draw
	                 : res == 0 ? RESULT_Black
	                            : RESULT_None;

	// bits 7-15 for for eco; 0->0, 1->A00 = 132, 2-> A01 = 263,...
	// ignore subcodes (bits 0-6)
	ecoT scidEco = (eco >> 7) ? ((eco >> 7) - 1) * 131 + 1 : 0;

	// game.Clear();
	game.gameDate = Date(year, month, day);
	game.whiteElo = white_rating & 0xFFF;
	game.blackElo = black_rating & 0xFFF;
	game.eco = scidEco;
	game.round = round;
	game.subround = subround;
	game.result = result;

	errorT err_player = pImpl->player_decoder->decode_record(
	    game, std::vector<uint32_t>{white_player, black_player});
	errorT err_tournament = pImpl->tournament_decoder->decode_record(
	    game, std::vector<uint32_t>{tournament});
	errorT err_annotator = pImpl->annotator_decoder->decode_record(
	    game, std::vector<uint32_t>{annotator});
	errorT err_source = pImpl->source_decoder->decode_record(
	    game, std::vector<uint32_t>{source});
	errorT err_game = pImpl->game_decoder->decode_record(
	    game, std::vector<uint32_t>{game_offset, annotation_offset});

	pImpl->n_parsed_ += 1;

	return err_player       ? err_player
	       : err_tournament ? err_tournament
	       : err_annotator  ? err_annotator
	       : err_source     ? err_source
	                        : err_game;
}

errorT CbhCodec::setGameIndex(uint32_t index) {
	if (index >= numGames())
		return ERROR_NotFound;
	pImpl->idxfile_.pubseekpos(INDEX_HEADER_SIZE + index * INDEX_ENTRY_SIZE);
	return OK;
}

size_t CbhCodec::numParsed() { return pImpl->n_parsed_; }

size_t CbhCodec::numGames() { return pImpl->n_games_; }

std::vector<std::string> CbhCodec::getFilenames() const {
	return pImpl->filenames_;
};

const char* CbhCodec::parseErrors() { return NULL; }

errorT CbhCodecImpl::read_index_header(const char* fname) {
	if (auto err = idxfile_.Open(fname, FMODE_ReadOnly))
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
