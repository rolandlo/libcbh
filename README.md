# libcbh

This is a library written in C++ for decoding chess databases in the cbh format, a widely used proprietary format by Chessbase. It is based on the implementation from [scidb](https://sourceforge.net/projects/scidb/) and contains some improvements and corrections over it.
For move decoding it uses part of [Scid](https://github.com/benini/scid/)'s code. Like scidb and scid this library is licensed under the GPL v2.

## Building the library

After cloning the repository and moving into the folder
```term
git clone https://github.com/rolandlo/libcbh.git
cd libcbh
```
the following lines will build the library in Release mode using the ninja generator.
```term
cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

Install it on your system using

```term
sudo ninja install
```

On Linux the library will go into

```term
/usr/local/lib
```
and the public headers into
```term
/usr/local/include/libcbh/
```

## Integrate the library into your C++-project

Using cmake add the following lines to your `CMakeLists.txt` file.

```cmake
find_library(CBH NAMES libcbh)
find_path(CBH_INCLUDE_PATH NAMES libcbh/cbh.h)
target_include_directories(your_target PRIVATE ${CBH_INCLUDE_PATH}/libcbh)
target_link_libraries(your_target PRIVATE cbh your_executables)
```
Replace `your_target` and `your_executables` appropriately.

## Basic usage

For a basic usage add the following lines into a C++-source file in your project:
```C++
#include "libcbh/cbh.h"

CbhCodec codec;

errorT err = codec.open("/path/to/your/database.cbh")
size_t numGames = codec.numGames();

for (int i = 0; i < numGames; i++) {
  GameReturnValue game;
  codec.parseNext(game);
  // Do something with game. 
  // See the public header interface.h about what members the GameReturnValue struct has.
}
```

## Stepping through annotated moves

For a `GameReturnValue game` the member `game.annotatedMoves` contains the moves of the chess game including annotations.
Each move is represented by

```C++
byte from;
byte to;
byte promote;
td::vector<Comment> comments;
```
Here the bytes `from` and `to` can take the values `0-63` and stand for the squares A1, B1, ..., H8 in that order.
The field `promote` takes 
- the value `EMPTY = 7` when there is no promotion
- the value `KING = 1` when it is a castling move (with `from` and `to` representing the squares for the King)
- the value `PAWN = 6` when it is a null move
- the value `byte(-1) = 255` when a new line starts to which we return once the current line is finished.
- the value `byte(-2) = 254` when a line ends.
- the value `byte(-3) = 253` for a skipped move. They are contained in rare cases and you can ignore those.

Step through the moves with a recursive function like in the following reference implementation in Scid:

```C++
uint32_t addAnnotatedMoves(std::vector<AnnotatedMove>& moves, Game& game, uint32_t start) {
       uint32_t i = start;
       while (i < moves.size()) {
               auto m = moves[i++];
               // read the move data into your move object
               simpleMoveT sm;               
               sm.from = m.from;
               sm.to = m.to;
               sm.promote = m.promote;
               sm.castling = 0;
               switch (m.promote) {
               case byte(-1): { // push
                        // save the current location for returning to it after the current line finishes
                        auto location = game.currentLocation();
                        i = addAnnotatedMoves(moves, game, i);
                        game.restoreLocation(location);
                        game.MoveForward();
                        game.AddVariation();
                        continue;
                };
                case byte(-2): { // pop
                        return i;
                }
                case byte(-3): { // skip
                        continue;
                }
                case PAWN: { // null move
                        sm.movingPiece = KING;
                        break;
                }
                case KING: { // castling
                        sm.promote = EMPTY;
                        sm.castling = 1;
                        break;
                }
                }
                // fill extra information into the move and add it to the game
                game.GetCurrentPos()->fillMove(sm);
                game.AddMove(sm);
                if (!m.comments.empty()) {
                        addAnnotations(m.comments, game);
                }
        }
        return i;
}
```
