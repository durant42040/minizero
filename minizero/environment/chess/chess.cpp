#include "chess.h"
#include <iostream>
#include <vector>

namespace minizero::env::chess {

    ChessAction::ChessAction(const std::vector <std::string> &action_string_args) {
        // split string into first two characters and last two characters
        assert(action_string_args.size() && action_string_args[0].size());
        const std::string action = action_string_args[0];

        const std::string start_pos = action.substr(0, 2);
        const std::string end_pos = action.substr(2, 2);
        const char promotion = (action.size() > 4) ? action[4] : 0;

        //    const int start_square = kChessPositions.find(start_pos);
        //    const int end_square = kChessPositions.find(end_pos);
    }

    std::string ChessAction::toConsoleString() const {
        // action id is encoded into numeric notation
        std::string action_string;
        int promotion = 0;

        const int start_square = action_id_ / kMovePerSquare;
        const int start_rank = start_square / kChessBoardSize;
        const int start_file = start_square % kChessBoardSize;

        int end_file = start_file;
        int end_rank = start_rank;

        const int move_id = action_id_ % kMovePerSquare;

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
            const int direction = (move_id - 64) / 3;  // 0: up-left, 1: up, 2: up-right
            promotion = (move_id - 64) % 3 + 1;        // 1: rook, 2: bishop, 3: queen
            end_file = end_file - 1 + direction;
            end_rank += 1;
            // TODO: Consider promotion to queen
        } else {
            std::cerr << "Invalid Move";
        }

        const int end_square = end_rank * kChessBoardSize + end_file;
        if (end_square < 0 || end_square >= kChessBoardSize * kChessBoardSize) {
            return "";
        }

        // action_string: "a1b1", action may be invalid
        action_string = kChessPositions[start_square] + kChessPositions[end_square] +
                        ((promotion == 0) ? "" : std::to_string(promotion));

        return action_string;
    }

    void ChessEnv::reset() {
        std::cout << "resetting..." << std::endl;
    }

    bool ChessEnv::act(const ChessAction &action) {
        std::cout << "acting..." << std::endl;
        return false;
    }

    bool ChessEnv::act(const std::vector <std::string> &action_string_args) {
        std::cout << "acting..." << std::endl;
        return false;
    }

    std::vector <ChessAction> ChessEnv::getLegalActions() const {
        std::cout << "getting legal actions..." << std::endl;
        return {ChessAction()};
    }

    bool ChessEnv::isLegalAction(const ChessAction &action) const {
        std::cout << "checking if action is legal..." << std::endl;
        return true;
    }

    bool ChessEnv::isTerminal() const {
        std::cout << "checking if terminal..." << std::endl;
        return false;
    }

    float ChessEnv::getEvalScore(bool is_resign) const {
        std::cout << "getting eval score..." << std::endl;
        return 1.0f;
    }

    std::string ChessEnv::toString() const {
        return "env.toString()";
    }

    std::vector<float> ChessEnv::getFeatures(utils::Rotation rotation) const {
        std::cout << "getting features..." << std::endl;
        return {};
    }

    std::vector<float> ChessEnv::getActionFeatures(const ChessAction &action, utils::Rotation rotation) const {
        std::cout << "getting action features..." << std::endl;
        return {};
    }

    std::vector<float> ChessEnvLoader::getFeatures(const int pos, utils::Rotation rotation) const {
        std::cout << "getting features..." << std::endl;
        return {};
    }

    std::vector<float> ChessEnvLoader::getActionFeatures(const int pos, utils::Rotation rotation) const {
        std::cout << "getting action features..." << std::endl;
        return {};
    }

} // namespace minizero::env::chess
