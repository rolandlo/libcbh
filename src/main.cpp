#include "codec_cbh.h"

int main(int argc, char* argv[]) {
	CodecCBH codec;
	const char* database = "/home/roland/Schach/PerformanceTest/PerformanceTest.cbh";
	codec.open(database);
	auto n_games = codec.numGames();
	printf("There are %ld games in %s\n", n_games, database);
	for (int i = 0; i < n_games; i++) {
		Game game;
		codec.parseNext(game);
		printf("%s - %s\n", game.GetWhiteStr(), game.GetBlackStr());
	}
	auto info = codec.parseProgress();
	printf("Parsed %ld games\n", info.first);
}
