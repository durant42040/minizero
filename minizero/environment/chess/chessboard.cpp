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
    // get castling rights
    result = result + "Castling rights: ";
    if (castling_rights_ & 1) {
        result += "K";
    }
    if (castling_rights_ & 2) {
        result += "Q";
    }
    if (castling_rights_ & 4) {
        result += "k";
    }
    if (castling_rights_ & 8) {
        result += "q";
    }

    return result;
}

bool ChessBoard::act(Square from, Square to, char promotion, bool update)
{
    updateDrawCondition(from, to);
    checkEnPassant(from, to);
    checkPromotion(promotion, from);
    Castling(from, to);
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
        // clear captured pawn
        if (white_pieces_.get(from)) {
            pawns_.clear(to.square_ - 8);
            black_pieces_.clear(to.square_ - 8);
            all_pieces_.clear(to.square_ - 8);
        } else if (black_pieces_.get(from)) {
            pawns_.clear(to.square_ + 8);
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

void ChessBoard::checkPromotion(char promotion, Square from)
{
    if (promotion) {
        switch (promotion) {
            case 'q': queens_.set(from); break;
            case 'r': rooks_.set(from); break;
            case 'b': bishops_.set(from); break;
            case 'n': knights_.set(from); break;
            default: std::cerr << "invalid promotion piece" << std::endl;
        }
        pawns_.clear(from);
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
    // add castling moves
    if (kings_.get(from)) {
        if (from == 4) {
            Bitboard all_black_moves = Bitboard(0);
            for (auto from : black_pieces_) {
                all_black_moves |= generateMoves(from);
            }

            // can castle if no squares occupied or attacked between king and rook, and castling rights are set
            bool can_white_castle_kingside =
                (all_black_moves & kWhiteKingsideSquares).empty() &&
                (all_pieces_ & (kWhiteKingsideSquares & ~kings_)).empty() &&
                (castling_rights_ & 1);
            bool can_white_castle_queenside =
                (all_black_moves & kWhiteQueensideSquares).empty() &&
                (all_pieces_ & (kWhiteQueensideSquares & ~kings_)).empty() &&
                (castling_rights_ & 2);

            if (can_white_castle_kingside) {
                moves.set(6);
            }
            if (can_white_castle_queenside) {
                moves.set(2);
            }
        } else if (from == 60) {
            Bitboard all_white_moves = Bitboard(0);
            for (auto from : white_pieces_) {
                all_white_moves |= generateMoves(from);
            }

            // can castle if no squares occupied or attacked between king and rook, and castling rights are set
            bool can_black_castle_kingside =
                (all_white_moves & kBlackKingsideSquares).empty() &&
                (all_pieces_ & (kBlackKingsideSquares & ~kings_)).empty() &&
                (castling_rights_ & 4);
            bool can_black_castle_queenside =
                (all_white_moves & kBlackQueensideSquares).empty() &&
                (all_pieces_ & (kBlackQueensideSquares & ~kings_)).empty() &&
                (castling_rights_ & 8);

            if (can_black_castle_kingside) {
                moves.set(62);
            }
            if (can_black_castle_queenside) {
                moves.set(58);
            }
        }
    }
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

void ChessBoard::Castling(Square from, Square to)
{
    // remove castling rights if king or rook is moved
    if (from == 0) {
        castling_rights_ &= ~2;
    } else if (from == 7) {
        castling_rights_ &= ~1;
    } else if (from == 4) {
        castling_rights_ &= ~3;
    } else if (from == 56) {
        castling_rights_ &= ~8;
    } else if (from == 60) {
        castling_rights_ &= ~12;
    } else if (from == 63) {
        castling_rights_ &= ~4;
    }

    // move rook if castling
    if (kings_.get(from) && abs(from.file_ - to.file_) == 2) {
        if (from == 4) {
            if (to == 2) {
                rooks_.update(0, 3);
                white_pieces_.update(0, 3);
                all_pieces_.update(0, 3);
            } else if (to == 6) {
                rooks_.update(7, 5);
                white_pieces_.update(7, 5);
                all_pieces_.update(7, 5);
            }
        } else if (from == 60) {
            if (to == 58) {
                rooks_.update(56, 59);
                black_pieces_.update(56, 59);
                all_pieces_.update(56, 59);
            } else if (to == 62) {
                rooks_.update(63, 61);
                black_pieces_.update(63, 61);
                all_pieces_.update(63, 61);
            }
        }
    }
}

} // namespace minizero::env::chess
