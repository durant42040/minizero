#include "chessboard.hpp"
#include "bitboard.hpp"
#include "move_generator.hpp"

#include <algorithm>
#include <string>

namespace minizero::env::chess {

std::string ChessBoard::toString(Bitboard bitboard) const
{
    std::string board; // Initialize an empty board with dots

    for (int i = 0; i < 64; i++) {
        if (pawns_.get(i)) {
            board += 'p';
        } else if (knights_.get(i)) {
            board += 'n';
        } else if (bishops_.get(i)) {
            board += 'b';
        } else if (rooks_.get(i)) {
            board += 'r';
        } else if (queens_.get(i)) {
            board += 'q';
        } else if (kings_.get(i)) {
            board += 'k';
        } else {
            board += '.';
        }
        if (black_pieces_.get(i)) {
            // convert last char to uppercase
            board.back() -= 32;
        }
        if (bitboard.get(i)) {
            board.back() = 'X';
        }
        if (i % 8 == 7) {
            board += '\n';
        } else {
            board += ' ';
        }
    }

    std::string result;
    for (int i = 7; i >= 0; i--) {
        result += board.substr(16 * i, 16);
    }

    return result;
}

bool ChessBoard::act(Square from, Square to)
{
    checkEnPassant(from, to);

    pawns_.update(from, to);
    knights_.update(from, to);
    bishops_.update(from, to);
    rooks_.update(from, to);
    queens_.update(from, to);
    kings_.update(from, to);
    white_pieces_.update(from, to);
    black_pieces_.update(from, to);
    all_pieces_.update(from, to);

    player_ = getNextPlayer(player_, 2);

    return true;
}

void ChessBoard::checkEnPassant(Square from, Square to)
{
    // check if en passant is played
    if (pawns_.get(from) && en_passant_.get(to)) {
        pawns_.clear(to.square_ - 8);
        if (player_ == Player::kPlayer1) {
            black_pieces_.clear(to.square_ - 8);
            all_pieces_.clear(to.square_ - 8);
        } else {
            white_pieces_.clear(to.square_ + 8);
            all_pieces_.clear(to.square_ + 8);
        }
    }
    en_passant_.reset();
    // check for double pawn moves
    if (pawns_.get(from) && (abs(from.rank_ - to.rank_) == 2)) {
        en_passant_.set((from.square_ + to.square_) / 2);
    }
}

Bitboard ChessBoard::generateMoves(Square from) const
{
    Bitboard moves(0);

    if (pawns_.get(from)) {
        if (player_ == Player::kPlayer1) {
            moves = generateWhitePawnMoves(from, all_pieces_, black_pieces_ | en_passant_);
        } else if (player_ == Player::kPlayer2) {
            moves = generateBlackPawnMoves(from, all_pieces_, white_pieces_ | en_passant_);
        }
    } else if (knights_.get(from)) {
        moves = generateKnightMoves(from);
    } else if (bishops_.get(from)) {
        moves = generateBishopMoves(from, all_pieces_);
    } else if (rooks_.get(from)) {
        moves = generateRookMoves(from, all_pieces_);
    } else if (queens_.get(from)) {
        moves = generateQueenMoves(from, all_pieces_);
    } else if (kings_.get(from)) {
        moves = generateKingMoves(from);
    } else {
        moves = Bitboard(0);
    }
    moves &= ~ourPieces();

    return moves;
}

} // namespace minizero::env::chess
