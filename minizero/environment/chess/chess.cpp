#include "chess.h"
#include "move_generator.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace minizero::env::chess {

std::vector<std::string> kChessActionName;
std::unordered_map<std::string, int> kChessActionMap;

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
        ChessAction action({kChessActionName[i]});
        assert(static_cast<int>(i) == action.getActionID());
    }
}

ChessAction::ChessAction(int action_id, Player player) : BaseAction(action_id, player)
{
    assert(action_id >= 0 && action_id < static_cast<int>(kChessActionName.size()));

    std::string action_string = kChessActionName[action_id];
    from_ = Square(action_string.substr(0, 2));
    to_ = Square(action_string.substr(2, 2));
}

ChessAction::ChessAction(const std::vector<std::string>& action_string_args)
{
    // construct action from action string
    assert(action_string_args.size() == 1);

    const std::string action_string = action_string_args[0];
    assert(kChessActionMap.count(action_string));

    action_id_ = kChessActionMap[action_string];
    from_ = Square(action_string.substr(0, 2));
    to_ = Square(action_string.substr(2, 2));
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
    } else if (move_id < 73) {
        // promotion
        if (start_rank != 6) return;
        const int direction = (move_id - 64) / 3; // 0: up-left, 1: up, 2: up-right

        const std::string promotion_pieces = "rbn";
        promotion = promotion_pieces[(move_id - 64) % 3]; // 0: rook, 1: bishop, 2: knight

        end_file = end_file - 1 + direction;
        end_rank += 1;

    } else {
        std::cerr << "Invalid Move";
    }

    // check bounds
    if (end_rank < 0 || end_rank > 7 || end_file < 0 || end_file > 7) return;

    // promotion to queen by default
    if (promotion == '\0' && start_rank == 6 && end_rank == 7 && abs(end_file - start_file) <= 1) {
        promotion = 'q';
    }

    const int end_square = end_rank * kChessBoardSize + end_file;

    // action_string: "a1b1" or "g7h7q"
    action_string = kChessPositions[start_square] + kChessPositions[end_square];
    if (promotion != '\0') {
        action_string += promotion;
    }

    kChessActionName.push_back(action_string);
    kChessActionMap[action_string] = kChessActionName.size() - 1;
}

std::string ChessAction::toConsoleString() const
{
    return kChessActionName[action_id_];
}

// reconstruct the board
void ChessEnv::reset()
{
    board_ = ChessBoard();
}

bool ChessEnv::act(const ChessAction& action)
{
    if (isLegalAction(action)) {
        board_.act(action.from_, action.to_);
        return true;
    } else {
        return false;
    }
}

// action_string_args: {action_string}, e.g. {"a1b1"}
bool ChessEnv::act(const std::vector<std::string>& action_string_args)
{
    ChessAction action = ChessAction(action_string_args);
    return act(action);
}

std::vector<ChessAction> ChessEnv::getLegalActions() const
{
    std::vector<ChessAction> legal_actions;
    std::cout << chessPlayerToString(board_.player_) << std::endl;

    for (auto from : board_.ourPieces()) {
        Bitboard moves = board_.generateMoves(from);

        for (auto to : moves) {
            std::string action_string = kChessPositions[from.square_] + kChessPositions[to.square_];
            legal_actions.push_back(ChessAction({action_string}));
        }
    }

    std::cout << "legal actions: " << legal_actions.size() << std::endl;

    return legal_actions;
}

bool ChessEnv::isLegalAction(const ChessAction& action) const
{
    Square from = action.from_;
    Square to = action.to_;

    if (board_.generateMoves(from).get(to) && board_.ourPieces().get(from)) {
        return true;
    } else {
        std::cout << "illegal move: " << action.toConsoleString() << std::endl;
        return false;
    }
}

bool ChessEnv::isTerminal() const
{
    std::cout << "checking if terminal..." << std::endl;
    return false;
}

float ChessEnv::getEvalScore(bool is_resign) const
{
    std::cout << "getting eval score..." << std::endl;
    return 1.0f;
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
    std::cout << "getting features..." << std::endl;
    return {};
}

std::vector<float> ChessEnv::getActionFeatures(const ChessAction& action, utils::Rotation rotation) const
{
    std::cout << "getting action features..." << std::endl;
    return {};
}

std::vector<float> ChessEnvLoader::getFeatures(const int pos, utils::Rotation rotation) const
{
    std::cout << "getting features..." << std::endl;
    return {};
}

std::vector<float> ChessEnvLoader::getActionFeatures(const int pos, utils::Rotation rotation) const
{
    std::cout << "getting action features..." << std::endl;
    return {};
}

} // namespace minizero::env::chess
