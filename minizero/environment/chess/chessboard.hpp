#pragma once

#include "base_env.h"
#include "bitboard.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace minizero::env::chess {

class ChessBoard {
public:
    ChessBoard()
        : player_(Player::kPlayer1),
          all_pieces_(kStartPos),
          white_pieces_(kWhiteStartPos),
          black_pieces_(kBlackStartPos),
          en_passant_(0),
          pawns_(kPawnStartPos),
          knights_(kKnightStartPos),
          bishops_(kBishopStartPos),
          rooks_(kRookStartPos),
          queens_(kQueenStartPos),
          kings_(kKingStartPos)
    {
    }

    std::string toString(Bitboard bitboard = 0) const;
    bool act(Square from, Square to);
    void checkEnPassant(Square from, Square to);
    Bitboard generateMoves(Square square) const;
    Bitboard ourPieces() const
    {
        return player_ == Player::kPlayer1 ? white_pieces_ : black_pieces_;
    }

    Player player_;

    Bitboard all_pieces_;

    Bitboard white_pieces_;
    Bitboard black_pieces_;

    Bitboard en_passant_;

    Bitboard pawns_;
    Bitboard knights_;
    Bitboard bishops_;
    Bitboard rooks_;
    Bitboard queens_;
    Bitboard kings_;
};

} // namespace minizero::env::chess
