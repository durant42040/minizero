#include "chess.h"
#include "move_generator.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace minizero::env::chess {

std::vector<std::string> kChessActionName;
std::unordered_map<std::string, int> kChessActionMap;
int kChessActionID[64][64];
int kPromotionActionID[64][64][4];

std::string chessPlayerToString(Player p)
{
    switch (p) {
        case Player::kPlayer1: return "W";
        case Player::kPlayer2: return "B";
        default: return "?";
    }
}

Player chessCharToPlayer(char c)
{
    switch (c) {
        case 'W':
        case 'w': return Player::kPlayer1;
        case 'B':
        case 'b': return Player::kPlayer2;
        default: return Player::kPlayerSize;
    }
}

void initialize()
{
    // compute action string and id mapping
    for (int i = 0; i < kMovePerSquare * kChessBoardSize * kChessBoardSize; i++) {
        generateActionString(i);
    }

    // check if the mapping is correct
    for (std::size_t i = 0; i < kChessActionName.size(); i++) {
        ChessAction action({"W", kChessActionName[i]});
        assert(static_cast<int>(i) == action.getActionID());
    }

    std::cout << kChessActionName.size() << " actions generated" << std::endl;
}

ChessAction::ChessAction(int action_id, Player player) : BaseAction(action_id, player)
{
    assert(action_id >= 0 && action_id < static_cast<int>(kChessActionName.size()));

    std::string action_string = kChessActionName[action_id];
    from_ = Square(action_string.substr(0, 2));
    to_ = Square(action_string.substr(2, 2));
    // if action string contains promotion, set promotion piece
    if (action_string.size() == 5) {
        promotion_ = action_string[4];
    } else {
        promotion_ = '\0';
    }
}

ChessAction::ChessAction(const std::vector<std::string>& action_string_args)
{
    // construct action from action string
    assert(action_string_args.size() == 2);
    assert(action_string_args[0].size() == 1);

    const std::string action_string = action_string_args[1];
    assert(kChessActionMap.count(action_string));

    action_id_ = kChessActionMap[action_string];
    player_ = chessCharToPlayer(action_string_args[0][0]);
    from_ = Square(action_string.substr(0, 2));
    to_ = Square(action_string.substr(2, 2));
    // if action string contains promotion, set promotion piece
    if (action_string.size() == 5) {
        promotion_ = action_string[4];
    } else {
        promotion_ = '\0';
    }
}

void generateActionString(int action_id)
{
    std::string action_string;
    char promotion = '\0';

    const int start_square = action_id / kMovePerSquare;
    const int start_rank = start_square / kChessBoardSize;
    const int start_file = start_square % kChessBoardSize;

    int end_file = start_file;
    int end_rank = start_rank;

    const int move_id = action_id % kMovePerSquare;

    if (move_id < 56) {
        // queen moves
        const int direction = move_id / 7;    // 0: N, 1: NE, 2: E, 3: SE, 4: S, 5: SW, 6: W, 7: NW
        const int distance = move_id % 7 + 1; // 1~7
        end_file += distance * ((direction % 4 == 0) ? 0 : ((direction > 4) ? -1 : 1));
        end_rank += distance * ((direction % 4 == 2) ? 0 : ((direction > 2 && direction < 6) ? -1 : 1));
    } else if (move_id < 64) {
        // knight moves
        if (move_id == 56) {
            end_file += 1;
            end_rank += 2;
        } else if (move_id == 57) {
            end_file += 2;
            end_rank += 1;
        } else if (move_id == 58) {
            end_file += 2;
            end_rank -= 1;
        } else if (move_id == 59) {
            end_file += 1;
            end_rank -= 2;
        } else if (move_id == 60) {
            end_file -= 1;
            end_rank -= 2;
        } else if (move_id == 61) {
            end_file -= 2;
            end_rank -= 1;
        } else if (move_id == 62) {
            end_file -= 2;
            end_rank += 1;
        } else {
            end_file -= 1;
            end_rank += 2;
        }
    } else if (move_id < 76) {
        // promotion
        if (start_rank == 6) {
            const int direction = (move_id - 64) / 4; // 0: up-left, 1: up, 2: up-right

            const std::string promotion_pieces = "qrbn";
            promotion = promotion_pieces[(move_id - 64) % 4]; // 0: queen, 1: rook, 2: bishop, 3: knight

            end_file = end_file - 1 + direction;
            end_rank += 1;
        } else if (start_rank == 1) {
            const int direction = (move_id - 64) / 4; // 0: down-left, 1: down, 2: down-right

            const std::string promotion_pieces = "qrbn";
            promotion = promotion_pieces[(move_id - 64) % 4]; // 0: queen, 1: rook, 2: bishop, 3: knight

            end_file = end_file - 1 + direction;
            end_rank -= 1;
        } else {
            return;
        }
    } else {
        std::cerr << "Invalid Move";
    }

    // check bounds
    if (end_rank < 0 || end_rank > 7 || end_file < 0 || end_file > 7) return;

    const int end_square = end_rank * kChessBoardSize + end_file;

    // action_string: "a1b1" or "g7h7q"
    action_string = kChessPositions[start_square] + kChessPositions[end_square];

    if (promotion == '\0') {
        kChessActionID[start_square][end_square] = kChessActionName.size();
    } else {
        action_string += promotion;
        kPromotionActionID[start_square][end_square][(move_id - 64) % 4] = kChessActionName.size();
    }

    kChessActionMap[action_string] = kChessActionName.size();
    kChessActionName.push_back(action_string);
}

std::string ChessAction::toConsoleString() const
{
    return kChessActionName[action_id_];
}

// reconstruct the board
void ChessEnv::reset()
{
    board_ = ChessBoard();
    actions_.clear();
    turn_ = Player::kPlayer1;
}

bool ChessEnv::act(const ChessAction& action)
{
    if (isLegalAction(action)) {
        actions_.push_back(action);
        turn_ = getNextPlayer(board_.player_, kChessNumPlayer);
        // default char with no promotion
        board_.act(action.from_, action.to_, action.promotion_);
        return true;
    } else {
        return false;
    }
}

// action_string_args: {player, action_string}, e.g. {"W", "a1b1"}
bool ChessEnv::act(const std::vector<std::string>& action_string_args)
{
    ChessAction action = ChessAction(action_string_args);
    return act(action);
}

std::vector<ChessAction> ChessEnv::getLegalActions() const
{
    std::vector<ChessAction> legal_actions;

    for (auto from : board_.ourPieces()) {
        Bitboard moves = board_.generateLegalMoves(from);

        for (auto to : moves) {
            if (board_.pawns_.get(from) && ((from.rank_ == 6 && to.rank_ == 7) || (from.rank_ == 1 && to.rank_ == 0))) {
                for (int i = 0; i < 4; i++) {
                    int promotion_action_id = kPromotionActionID[from.square_][to.square_][i];
                    legal_actions.push_back(ChessAction(promotion_action_id, board_.player_));
                }
            } else {
                int action_id = kChessActionID[from.square_][to.square_];
                legal_actions.push_back(ChessAction(action_id, board_.player_));
            }
        }
    }

    return legal_actions;
}

bool ChessEnv::isLegalAction(const ChessAction& action) const
{
    Square from = action.from_;
    Square to = action.to_;

    if (board_.generateLegalMoves(from).get(to) && board_.ourPieces().get(from)) {
        return true;
    } else {
        std::cout << "illegal move: " << action.toConsoleString() << std::endl;
        return false;
    }
}

bool ChessEnv::isTerminal() const
{
    switch (board_.game_state_) {
        case GameState::Playing:
            std::cout << "playing" << std::endl;
            break;
        case GameState::WhiteWin:
            std::cout << "white win" << std::endl;
            break;
        case GameState::BlackWin:
            std::cout << "black win" << std::endl;
            break;
        case GameState::Draw:
            std::cout << "draw" << std::endl;
            break;
    }
    return board_.game_state_ != GameState::Playing;
}

float ChessEnv::getEvalScore(bool is_resign) const
{
    if (board_.game_state_ == GameState::Draw) {
        return 0.0f;
    } else if (board_.game_state_ == GameState::WhiteWin) {
        return 1.0f;
    } else if (board_.game_state_ == GameState::BlackWin) {
        return -1.0f;
    } else {
        return 0.0f;
    }
}

std::string ChessEnv::toString() const
{
    return board_.toString();
}

std::string ChessEnv::DebugBitboardString(Bitboard bitboard) const
{
    return board_.toString(bitboard);
}

std::vector<float> ChessEnv::getFeatures(utils::Rotation rotation) const
{
    std::vector<float> features;
    for (int i = 0; i < 64; i++) {
        features.push_back(board_.pawns_.get(i));
    }
    for (int i = 0; i < 64; i++) {
        features.push_back(board_.bishops_.get(i));
    }
    for (int i = 0; i < 64; i++) {
        features.push_back(board_.rooks_.get(i));
    }
    for (int i = 0; i < 64; i++) {
        features.push_back(board_.queens_.get(i));
    }
    for (int i = 0; i < 64; i++) {
        features.push_back(board_.kings_.get(i));
    }
    for (int i = 0; i < 64; i++) {
        features.push_back(board_.knights_.get(i));
    }
    for (int i = 0; i < 64; i++) {
        features.push_back(board_.white_pieces_.get(i));
    }
    for (int i = 0; i < 64; i++) {
        features.push_back(board_.black_pieces_.get(i));
    }
    for (int i = 0; i < 64; i++) {
        features.push_back(board_.player_ == Player::kPlayer1 ? 1.0f : 0.0f);
    }
    return features;
}

std::vector<float> ChessEnv::getActionFeatures(const ChessAction& action, utils::Rotation rotation) const
{
    std::cout << "getting action features..." << std::endl;
    return {};
}


std::vector<float> ChessEnvLoader::getActionFeatures(const int pos, utils::Rotation rotation) const
{
    std::cout << "getting action features..." << std::endl;
    return {};
}

} // namespace minizero::env::chess
