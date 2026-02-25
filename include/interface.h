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

#pragma once

#include <string>
#include <variant>
#include <vector>

using byte = unsigned char;
using eloT = ushort;
using ecoT = ushort;
using langT = ushort;
using nagT = byte;

struct TextBeforeComment {
	langT lang;
	std::string text;
	TextBeforeComment(langT lang, std::string text)
	    : lang(lang), text(text) {};
};

struct TextAfterComment {
	langT lang;
	std::string text;
	TextAfterComment(langT lang, std::string text)
	    : lang(lang), text(text) {};
};

struct ArrowComment {
	byte from;
	byte to;
	std::string color;
	ArrowComment() = default;
	ArrowComment(byte from, byte to, std::string color): from(from), to(to), color(color) {};
};

struct SquareComment {
	byte sq;
	std::string color;
	SquareComment() = default;
	SquareComment(byte sq, std::string color): sq(sq), color(color) {}
};

struct SymbolComment {
	nagT symbol = 0;
	nagT evaluation = 0;
	nagT prefix = 0;
	SymbolComment() = default;
	SymbolComment(nagT symbol, nagT evaluation, nagT prefix)
	    : symbol(symbol), evaluation(evaluation), prefix(prefix) {};
};

using Comment = std::variant<TextBeforeComment, TextAfterComment, ArrowComment,
                             SquareComment, SymbolComment>;

struct AnnotatedMove {
	byte from;
	byte to;
	byte promote;
	std::vector<Comment> comments;
	AnnotatedMove() = default;
	AnnotatedMove(byte from, byte to, byte promote,
	              std::vector<Comment> comments)
	    : from(from), to(to), promote(promote), comments(comments) {}
};

static auto MovePush = AnnotatedMove(0, 0, byte(-1), std::vector<Comment>());
static auto MovePop = AnnotatedMove(0, 0, byte(-2), std::vector<Comment>());
static auto MoveSkip = AnnotatedMove(0, 0, byte(-3), std::vector<Comment>());

struct Tag {
	std::string tag;
	std::string value;
	Tag() = default;
	Tag(std::string tag, std::string value)
	    : tag(tag), value(value) {};
};

struct Date {
	uint year;
	uint month;
	uint day;
	Date() = default;
	Date(uint year, uint month, uint day)
	    : year(year), month(month), day(day) {};
};

struct GameReturnValue {
	std::vector<AnnotatedMove> annotatedMoves;
	std::vector<Tag> tags;
	std::string event;
	Date gameDate;
	eloT whiteElo;
	eloT blackElo;
	std::string whiteFirstName;
	std::string whiteName;
	std::string blackFirstName;
	std::string blackName;
	ecoT eco;
	byte round;
	byte subround;
	byte result;
	std::string eventTitle;
	std::string eventPlace;
	Date eventDate;
	std::string startFen;
	GameReturnValue() = default;
};
