#pragma once

#include "base_env.h"
#include "bitboard.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace minizero::env::chess {

enum class GameState {
    Playing,
    WhiteWin,
    BlackWin,
    Draw,
};

class ChessBoard {
public:
    ChessBoard()
        : game_state_(GameState::Playing),

          player_(Player::kPlayer1),
          fifty_move_rule_(0),
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
    bool act(Square from, Square to, char promotion, bool update = true);
    void checkEnPassant(Square from, Square to);
    bool isPlayerInCheck(Player player) const;
    // generate pseudolegal moves
    Bitboard generateMoves(Square square) const;
    // remove moves from generateMoves that would leave the king in check
    Bitboard generateLegalMoves(Square square) const;
    void updateGameState();
    void updateDrawCondition(Square from, Square to);

    inline Bitboard ourPieces(Player player) const
    {
        return player == Player::kPlayer1 ? white_pieces_ : black_pieces_;
    }
    inline Bitboard ourPieces() const
    {
        return ourPieces(player_);
    }
    inline Bitboard theirPieces(Player player) const
    {
        return player == Player::kPlayer1 ? black_pieces_ : white_pieces_;
    }
    inline Bitboard theirPieces() const
    {
        return theirPieces(player_);
    }
    bool hasMatingMaterial() const;

    GameState game_state_;
    Player player_;
    int fifty_move_rule_;

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
