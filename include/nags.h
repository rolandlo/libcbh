#pragma once

using byte = unsigned char;
/*
 * Common NAG (Numeric annotation glyph) Annotation symbol values. See
 * https://en.wikipedia.org/wiki/Portable_Game_Notation#Numeric_Annotation_Glyphs_(NAGs)
 * For standard NAGS (from the png standard) see also
 * https://ia802908.us.archive.org/26/items/pgn-standard-1994-03-12/PGN_standard_1994-03-12.txt
 */
const byte NAG_GoodMove = 1,                    // standard
    NAG_PoorMove = 2,                           // standard
    NAG_ExcellentMove = 3,                      // standard
    NAG_Blunder = 4,                            // standard
    NAG_InterestingMove = 5,                    // standard
    NAG_DubiousMove = 6,                        // standard
    NAG_OnlyMove = 8,                           // standard
    NAG_Equal = 10,                             // standard
    NAG_Unclear = 13,                           // standard
    NAG_WhiteSlight = 14,                       // standard
    NAG_BlackSlight = 15,                       // standard
    NAG_WhiteClear = 16,                        // standard
    NAG_BlackClear = 17,                        // standard
    NAG_WhiteDecisive = 18,                     // standard
    NAG_BlackDecisive = 19,                     // standard
    NAG_WhiteCrushing = 20,                     // standard
    NAG_BlackCrushing = 21,                     // standard
    NAG_WhiteZugZwang = 22,                     // standard
    NAG_BlackZugZwang = 23,                     // standard
    NAG_WhiteMoreRoom = 26,                     // standard
    NAG_BlackMoreRoom = 27,                     // standard
    NAG_WhiteModerateDevelopmentAdvantage = 32, // standard, missing in scid
    NAG_BlackModerateDevelopmentAdvantage = 33, // standard, missing in scid
    NAG_WhiteDevelopmentAdvantage = 34,         // standard, missing in scid
    NAG_BlackDevelopmentAdvantage = 35,         // standard
    NAG_WhiteWithInitiative = 36,               // standard
    NAG_BlackWithInitiative = 37,               // standard, missing in scid
    NAG_WhiteWithAttack = 40,                   // standard
    NAG_BlackWithAttack = 41,                   // standard
    NAG_WhiteCompensation = 44,                 // standard
    NAG_BlackCompensation = 45,                 // standard, missing in scid
    NAG_WhiteSlightCentre = 48,                 // standard
    NAG_BlackSlightCentre = 49,                 // standard, missing in scid
    NAG_WhiteCentre = 50,                       // standard
    NAG_BlackCentre = 51,                       // standard, missing in scid
    NAG_WhiteSlightKingSide = 54,               // standard
    NAG_BlackSlightKingSide = 55,               // standard, missing in scid
    NAG_WhiteModerateKingSide = 56,             // standard
    NAG_BlackModerateKingSide = 57,             // standard, missing in scid
    NAG_WhiteKingSide = 58,                     // standard
    NAG_BlackKingSide = 59,                     // standard, missing in scid
    NAG_WhiteSlightQueenSide = 60,              // standard
    NAG_BlackSlightQueenSide = 61,              // standard, missing in scid
    NAG_WhiteModerateQueenSide = 62,            // standard
    NAG_BlackModerateQueenSide = 63,            // standard, missing in scid
    NAG_WhiteQueenSide = 64,                    // standard
    NAG_BlackQueenSide = 65,                    // standard, missing in scid
    NAG_WhiteSlightCounterPlay = 130,           // standard
    NAG_BlackSlightCounterPlay = 131,           // standard
    NAG_WhiteCounterPlay = 132,                 // standard
    NAG_BlackCounterPlay = 133,                 // standard
    NAG_WhiteDecisiveCounterPlay = 134,         // standard
    NAG_BlackDecisiveCounterPlay = 135,         // standard
    NAG_WhiteTimeControlPressure = 136,         // standard
    NAG_BlackTimeControlPressure = 137,         // standard, missing in scid
    NAG_WithIdea = 140,                         // ChessPad
    NAG_AimedAgainst = 141,                     // ChessPad, missing in scid
    NAG_BetterIs = 142,                         // ChessPad
    NAG_WorseIs = 143,                          // ChessPad
    NAG_EquivalentIs = 144,                     // ChessPad
    NAG_EditorialComment = 145,                 // ChessPad
    NAG_Novelty = 146;                          // ChessPad
