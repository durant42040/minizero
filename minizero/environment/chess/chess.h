#pragma once

#include "base_env.h"

namespace minizero::env::chess {

const std::string kChessName = "chess";
const int kChessNumPlayer = 2;
const int kChessBoardSize = 8;
const int kMovePerSquare = 73;
const int kChessPieces = 12;

const std::string kChessPositions[] = {
    "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8",
    "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8",
    "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8",
    "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8",
    "e1", "e2", "e3", "e4", "e5", "e6", "e7", "e8",
    "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8",
    "g1", "g2", "g3", "g4", "g5", "g6", "g7", "g8",
    "h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8"
};

//      CHESSBOARD
// 8 56 57 58 59 60 61 62 63
// 7 48 49 50 51 52 53 54 55
// 6 40 41 42 43 44 45 46 47
// 5 32 33 34 35 36 37 38 39
// 4 24 25 26 27 28 29 30 31
// 3 16 17 18 19 20 21 22 23
// 2 8  9  10 11 12 13 14 15
// 1 0  1  2  3  4  5  6  7
//   a  b  c  d  e  f  g  h

class ChessAction : public BaseAction {
public:
    ChessAction() : BaseAction() {}
    ChessAction(int action_id, Player player) : BaseAction(action_id, player) {}
    explicit ChessAction(const std::vector<std::string>& action_string_args);
    inline Player nextPlayer() const override { return getNextPlayer(getPlayer(), kChessNumPlayer); }
    std::string toConsoleString() const override;
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
    inline int getNumActionFeatureChannels() const override { return 0; } // TODO
    inline int getInputChannelHeight() const override { return kChessPieces; }
    inline int getInputChannelWidth() const override { return getBoardSize() * getBoardSize(); }
    inline int getHiddenChannelHeight() const override { return kChessPieces; }
    inline int getHiddenChannelWidth() const override { return getBoardSize() * getBoardSize(); }
    inline int getPolicySize() const override { return getBoardSize() / 2 * 12; }
    std::string toString() const override;
    inline std::string name() const override { return kChessName + std::to_string(getBoardSize()) + "x" + std::to_string(getBoardSize()); }
    inline int getNumPlayer() const override { return kChessNumPlayer; }

    inline int getRotatePosition(int position, utils::Rotation rotation) const override { return utils::getPositionByRotating(utils::Rotation::kRotationNone, position, getBoardSize()); };
    inline int getRotateAction(int action_id, utils::Rotation rotation) const override { return getRotatePosition(action_id, utils::Rotation::kRotationNone); };
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
