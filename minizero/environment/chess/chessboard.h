#pragma once

#include "base_env.h"
#include "bitboard.h"
#include "move_generator.h"
#include <iostream>
#include <string>
#include <vector>

namespace minizero::env::chess {

extern uint64_t PieceKeys[2][6][64];
extern uint64_t CastleKeys[4];
extern uint64_t EnPassantKeys[8];
extern uint64_t whiteToMoveKey;

const uint64_t kWhiteKingsideSquares = 0x0000000000000070;
const uint64_t kWhiteQueensideSquares = 0x000000000000001C;
const uint64_t kBlackKingsideSquares = 0x7000000000000000;
const uint64_t kBlackQueensideSquares = 0x1C00000000000000;
const std::string kStartingFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

enum class GameState {
    Playing,
    WhiteWin,
    BlackWin,
    Draw,
};

void initKeys();

class ChessBoard {
public:
    ChessBoard() : game_state_(GameState::Playing)
    {
        setFen(kStartingFen);
    }

    ChessBoard(std::string fen) : game_state_(GameState::Playing)
    {
        setFen(fen);
    }

    std::string toString(Square prev_move_from, Square prev_move_to) const;
    bool act(Square from, Square to, char promotion, bool update = true);
    void checkEnPassant(Square from, Square to);
    void checkPromotion(char promotion, Square from);
    bool hasMatingMaterial() const;
    bool isPlayerInCheck(Player player) const;
    // generate pseudolegal moves
    Bitboard generateMoves(Square square) const;
    // remove moves from generateMoves that would leave the king in check
    Bitboard generateLegalMoves(Square square) const;
    std::vector<uint64_t> getPositionInfo() const;
    void updateGameState();
    void updateDrawCondition(Square from, Square to);
    void castling(Square from, Square to);
    void setFen(std::string fen);
    uint64_t generateHash() const;
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
    inline int getRepetitionCount() const
    {
        int repetitions = 0;
        for (int i = position_history_.size() - 3; i >= 0; i -= 2) {
            if (position_history_[i] == position_history_.back()) {
                repetitions++;
            }
        }
        return repetitions;
    }

    GameState game_state_;
    Player player_;
    std::vector<uint64_t> position_history_;
    int fifty_move_rule_;
    int fullmove_number_;
    int castling_rights_;
    Bitboard en_passant_;

    Bitboard all_pieces_;

    Bitboard white_pieces_;
    Bitboard black_pieces_;

    Bitboard pawns_;
    Bitboard knights_;
    Bitboard bishops_;
    Bitboard rooks_;
    Bitboard queens_;
    Bitboard kings_;
};

} // namespace minizero::env::chess