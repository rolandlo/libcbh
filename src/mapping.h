#pragma once

#include "board_def.h"
using byte = unsigned char;

// Chessbase enumerates squares from a1 to a8 then b1 to b8 etc.
static inline squareT mapSquare(byte sq) {
	return square_Make(sq >> 3, sq & 7);
}
