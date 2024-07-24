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

    result = result + "Player: " + (player_ == Player::kPlayer1 ? "White" : "Black") + "\n";
    result = result + "In Check: " + (isPlayerInCheck(player_) ? "Yes" : "No") + "\n";

    return result;
}

bool ChessBoard::act(Square from, Square to, char promotion, bool update)
{
    updateDrawCondition(from, to);
    checkEnPassant(from, to);
    if (promotion) {
        switch (promotion) {
            case 'q': queens_.set(from); break;
            case 'r': rooks_.set(from); break;
            case 'b': bishops_.set(from); break;
            case 'n': knights_.set(from); break;
            default: std::cerr << "invalid promotion piece" << std::endl; return false;
        }
        pawns_.clear(from);
    }
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

    if (update) {
        updateGameState();
    }

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
    Bitboard our_pieces = white_pieces_.get(from) ? white_pieces_ : black_pieces_;
    Bitboard moves(0);

    if (pawns_.get(from)) {
        if (white_pieces_.get(from)) {
            moves = generateWhitePawnMoves(from, all_pieces_, black_pieces_ | en_passant_);
        } else if (black_pieces_.get(from)) {
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
        // TODO: Castling
    } else {
        moves = Bitboard(0);
    }

    // Remove moves that would capture our own pieces
    moves &= ~our_pieces;

    return moves;
}

Bitboard ChessBoard::generateLegalMoves(Square from) const
{
    Bitboard moves = generateMoves(from);
    // remove moves that would put our king in check
    // e.g. pins, illegal king moves
    for (auto to : moves) {
        ChessBoard temp_board = *this;
        temp_board.act(from, to, '\0', false);
        if (temp_board.isPlayerInCheck(player_)) {
            moves.clear(to);
        }
    }
    return moves;
}

bool ChessBoard::isPlayerInCheck(Player player) const
{
    Bitboard their_pieces = theirPieces(player);
    Bitboard our_king = ourPieces(player) & kings_;
    Square our_king_position = Square(our_king.getLSB());

    for (auto from : their_pieces) {
        Bitboard moves = generateMoves(from);
        if (moves.get(our_king_position)) {
            return true;
        }
    }
    return false;
}

void ChessBoard::updateGameState()
{
    Bitboard all_moves = Bitboard(0);
    for (auto from : ourPieces()) {
        all_moves |= generateLegalMoves(from);
    }
    if (all_moves.empty()) {
        if (isPlayerInCheck(player_)) {
            // Checkmate
            if (player_ == Player::kPlayer1) {
                game_state_ = GameState::BlackWin;
            } else if (player_ == Player::kPlayer2) {
                game_state_ = GameState::WhiteWin;
            }
        } else {
            // Stalemate
            game_state_ = GameState::Draw;
        }
    }
    // Insufficient material
    if (!hasMatingMaterial()) {
        game_state_ = GameState::Draw;
    }
    // 50-move rule
    if (fifty_move_rule_ == 50) {
        game_state_ = GameState::Draw;
    }
    // TODO: threefold-repetition
}

bool ChessBoard::hasMatingMaterial() const
{
    if (!pawns_.empty() || !rooks_.empty() || !queens_.empty()) {
        return true;
    }

    int num_white_bishops = (bishops_ & white_pieces_).count();
    int num_black_bishops = (bishops_ & black_pieces_).count();
    int num_white_knights = (knights_ & white_pieces_).count();
    int num_black_knights = (knights_ & black_pieces_).count();

    if (num_white_bishops + num_white_knights > 1 || num_black_bishops + num_black_knights > 1) {
        return true;
    }

    return false;
}

void ChessBoard::updateDrawCondition(Square from, Square to)
{
    fifty_move_rule_++;
    // reset if pawn is moved or piece is captured
    if (pawns_.get(from) || all_pieces_.get(to)) {
        fifty_move_rule_ = 0;
    }
}

} // namespace minizero::env::chess
