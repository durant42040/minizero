#pragma once

#include "base_env.h"
#include "bitboard.hpp"
#include "chessboard.hpp"
#include "move_generator.hpp"
#include "square.hpp"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace minizero::env::chess {

const std::string kChessName = "chess";
const int kChessNumPlayer = 2;
const int kChessBoardSize = 8;
const int kMovePerSquare = 73;
const int kChessPieces = 12;
extern std::vector<std::string> kChessActionName;
extern std::unordered_map<std::string, int> kChessActionMap;

const std::string kChessPositions[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

void initialize();
void generateActionString(int action_id);

class ChessAction : public BaseAction {
public:
    ChessAction() : BaseAction() {}
    ChessAction(int action_id, Player player);
    explicit ChessAction(const std::vector<std::string>& action_string_args);
    inline Player nextPlayer() const override { return getNextPlayer(getPlayer(), kChessNumPlayer); }
    std::string toConsoleString() const override;

    Square from_;
    Square to_;
};

class ChessEnv : public BaseBoardEnv<ChessAction> {
public:
    ChessEnv() : BaseBoardEnv<ChessAction>(kChessBoardSize)
    {
        reset();
    }

    void reset() override;
    bool act(const ChessAction& action) override;
    bool act(const std::vector<std::string>& action_string_args) override;
    std::vector<ChessAction> getLegalActions() const override;
    bool isLegalAction(const ChessAction& action) const override;
    bool isTerminal() const override;
    float getReward() const override { return 0.0f; }

    float getEvalScore(bool is_resign = false) const override;
    std::vector<float> getFeatures(utils::Rotation rotation = utils::Rotation::kRotationNone) const override;
    std::vector<float> getActionFeatures(const ChessAction& action, utils::Rotation rotation = utils::Rotation::kRotationNone) const override;
    inline int getNumInputChannels() const override { return kChessPieces; }
    inline int getNumActionFeatureChannels() const override { return 0; }
    inline int getInputChannelHeight() const override { return kChessPieces; }
    inline int getInputChannelWidth() const override { return getBoardSize() * getBoardSize(); }
    inline int getHiddenChannelHeight() const override { return kChessPieces; }
    inline int getHiddenChannelWidth() const override { return getBoardSize() * getBoardSize(); }
    inline int getPolicySize() const override { return getBoardSize() / 2 * 12; }
    std::string toString() const override;
    std::string DebugBitboardString(Bitboard bitboard) const;
    inline std::string name() const override { return kChessName + std::to_string(getBoardSize()) + "x" + std::to_string(getBoardSize()); }
    inline int getNumPlayer() const override { return kChessNumPlayer; }

    inline int getRotatePosition(int position, utils::Rotation rotation) const override { return utils::getPositionByRotating(utils::Rotation::kRotationNone, position, getBoardSize()); };
    inline int getRotateAction(int action_id, utils::Rotation rotation) const override { return getRotatePosition(action_id, utils::Rotation::kRotationNone); };

    ChessBoard board_;
};

class ChessEnvLoader : public BaseBoardEnvLoader<ChessAction, ChessEnv> {
public:
    void loadFromEnvironment(const ChessEnv& env, const std::vector<std::vector<std::pair<std::string, std::string>>>& action_info_history = {}) override {}
    inline int getSeed() const { return 0; }

    std::vector<float> getFeatures(const int pos, utils::Rotation rotation = utils::Rotation::kRotationNone) const override;
    std::vector<float> getActionFeatures(const int pos, utils::Rotation rotation = utils::Rotation::kRotationNone) const override;
    inline std::vector<float> getValue(const int pos) const { return {getReturn()}; }
    inline std::string name() const override { return kChessName + std::to_string(getBoardSize()) + "x" + std::to_string(getBoardSize()); }
    inline int getPolicySize() const override { return getBoardSize() / 2 * 12; }
    inline int getRotatePosition(int position, utils::Rotation rotation) const override { return utils::getPositionByRotating(utils::Rotation::kRotationNone, position, getBoardSize()); }
    inline int getRotateAction(int action_id, utils::Rotation rotation) const override { return getRotatePosition(action_id, utils::Rotation::kRotationNone); }
};

} // namespace minizero::env::chess
