#pragma once

#include "square.hpp"
#include "bitboard.hpp"
namespace minizero::env::chess {

extern const Bitboard kWhitePawnCaptures[64];
extern const Bitboard kBlackPawnCaptures[64];
extern const Bitboard kKnightAttacks[64];
extern const Bitboard kKingAttacks[64];

Bitboard generateWhitePawnMoves(Square from, Bitboard allPieces, Bitboard allBlackPieces);

Bitboard generateBlackPawnMoves(Square from, Bitboard allPieces, Bitboard allWhitePieces);

Bitboard generateKnightMoves(Square from);

Bitboard generateKingMoves(Square from);

Bitboard generateRookMoves(Square from, Bitboard all_pieces);

Bitboard generateBishopMoves(Square from, Bitboard all_pieces);

Bitboard generateQueenMoves(Square from, Bitboard all_pieces);

}

