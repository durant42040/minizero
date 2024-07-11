#include "chess.h"
#include <iostream>
#include <string>
#include <vector>

namespace minizero::env::chess {

std::vector<std::string> kChessActionName;
std::unordered_map<std::string, int> kChessActionMap;

void initialize()
{
    // compute action string and id mapping
    for (int i = 0; i < kMovePerSquare * kChessBoardSize * kChessBoardSize; i++) {
        generateActionString(i);
    }

    // check if the mapping is correct
    for(std::size_t i = 0; i < kChessActionName.size(); i++) {
        ChessAction action({kChessActionName[i]});
        assert(static_cast<int>(i) == action.getActionID());
    }
}

ChessAction::ChessAction(const std::vector<std::string>& action_string_args)
{
    // construct action from action string
    assert(action_string_args.size() && action_string_args[0].size());
    const std::string action_string = action_string_args[0];
    action_id_ = kChessActionMap[action_string];
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
        if(start_rank != 6) return;
        const int direction = (move_id - 64) / 3;  // 0: up-left, 1: up, 2: up-right

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
    action_string = kChessPositions[start_square] + kChessPositions[end_square] + promotion;

    kChessActionName.push_back(action_string);
    kChessActionMap[action_string] = kChessActionName.size() - 1;
}

std::string ChessAction::toConsoleString() const
{
    return kChessActionName[action_id_];
}

void ChessEnv::reset()
{
    std::cout << "resetting..." << std::endl;
}

bool ChessEnv::act(const ChessAction& action)
{
    std::cout << "acting..." << std::endl;
    return false;
}

bool ChessEnv::act(const std::vector<std::string>& action_string_args)
{
    std::cout << "acting..." << std::endl;
    return false;
}

std::vector<ChessAction> ChessEnv::getLegalActions() const
{
    std::cout << "getting legal actions..." << std::endl;
    return {ChessAction()};
}

bool ChessEnv::isLegalAction(const ChessAction& action) const
{
    std::cout << "checking if action is legal..." << std::endl;
    return true;
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
    return "env.toString()";
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
