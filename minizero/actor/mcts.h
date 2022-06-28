#pragma once

#include "configuration.h"
#include "environment.h"
#include "random.h"
#include "tree.h"
#include <cmath>
#include <map>
#include <vector>

namespace minizero::actor {

class MCTSNode : public BaseTreeNode {
public:
    MCTSNode() { reset(); }

    void reset() override
    {
        num_children_ = 0;
        extra_data_index_ = -1;
        mean_ = 0.0f;
        count_ = 0.0f;
        policy_ = 0.0f;
        policy_logit_ = 0.0f;
        policy_noise_ = 0.0f;
        value_ = 0.0f;
        reward_ = 0.0f;
        first_child_ = nullptr;
    }

    void add(float value, float weight = 1.0f)
    {
        if (count_ + weight <= 0) {
            reset();
        } else {
            count_ += weight;
            mean_ += weight * (value - mean_) / count_;
        }
    }

    void remove(float value, float weight = 1.0f)
    {
        if (count_ + weight <= 0) {
            reset();
        } else {
            count_ -= weight;
            mean_ -= weight * (value - mean_) / count_;
        }
    }

    virtual float getNormalizedMean(const std::map<float, int>& tree_value_map) const
    {
        float value = mean_;
        if (config::actor_mcts_value_rescale) {
            if (tree_value_map.size() < 2) { return 1.0f; }
            const float value_lower_bound = tree_value_map.begin()->first;
            const float value_upper_bound = tree_value_map.rbegin()->first;
            value = (mean_ - value_lower_bound) / (value_upper_bound - value_lower_bound);
            value = fmin(1, fmax(-1, 2 * value - 1)); // normalize to [-1, 1]
        }
        // flip value according to player
        value = (action_.getPlayer() == env::Player::kPlayer1 ? value : -value);
        return value;
    }

    virtual float getNormalizedPUCTScore(int total_simulation, const std::map<float, int>& tree_value_map, float init_q_value = -1.0f) const
    {
        float puct_bias = config::actor_mcts_puct_init + log((1 + total_simulation + config::actor_mcts_puct_base) / config::actor_mcts_puct_base);
        float value_u = (puct_bias * getPolicy() * sqrt(total_simulation)) / (1 + count_);
        float value_q = (count_ == 0 ? init_q_value : getNormalizedMean(tree_value_map));
        return value_u + value_q;
    }

    std::string toString() const override
    {
        std::ostringstream oss;
        oss.precision(4);
        oss << std::fixed << "p = " << policy_
            << ", p_logit = " << policy_logit_
            << ", p_noise = " << policy_noise_
            << ", v = " << value_
            << ", r = " << reward_
            << ", mean = " << mean_
            << ", count = " << count_;
        return oss.str();
    }

    bool displayInTreeLog() const override { return count_ > 0; }

    // setter
    inline void setExtraDataIndex(int extra_data_index) { extra_data_index_ = extra_data_index; }
    inline void setMean(float mean) { mean_ = mean; }
    inline void setCount(float count) { count_ = count; }
    inline void setPolicy(float policy) { policy_ = policy; }
    inline void setPolicyLogit(float policy_logit) { policy_logit_ = policy_logit; }
    inline void setPolicyNoise(float policy_noise) { policy_noise_ = policy_noise; }
    inline void setValue(float value) { value_ = value; }
    inline void setReward(float reward) { reward_ = reward; }
    inline void setFirstChild(MCTSNode* first_child) { BaseTreeNode::setFirstChild(first_child); }

    // getter
    inline int getExtraDataIndex() const { return extra_data_index_; }
    inline float getMean() const { return mean_; }
    inline float getCount() const { return count_; }
    inline float getPolicy() const { return policy_; }
    inline float getPolicyLogit() const { return policy_logit_; }
    inline float getPolicyNoise() const { return policy_noise_; }
    inline float getValue() const { return value_; }
    inline float getReward() const { return reward_; }
    inline MCTSNode* getFirstChild() const { return static_cast<MCTSNode*>(BaseTreeNode::getFirstChild()); }

protected:
    int extra_data_index_;
    float mean_;
    float count_;
    float policy_;
    float policy_logit_;
    float policy_noise_;
    float value_;
    float reward_;
};

class MCTSNodeExtraData {
public:
    MCTSNodeExtraData(const std::vector<float>& hidden_state)
        : hidden_state_(hidden_state) {}
    std::vector<float> hidden_state_;
};

template <class Node, class Tree, class TreeExtraData>
class BaseMCTS {
public:
    class ActionCandidate {
    public:
        Action action_;
        float policy_;
        float policy_logit_;
        ActionCandidate(const Action& action, const float& policy, const float& policy_logit)
            : action_(action), policy_(policy), policy_logit_(policy_logit) {}
    };

    BaseMCTS(long long tree_node_size)
        : tree_(tree_node_size) {}

    virtual void reset()
    {
        tree_.reset();
        tree_extra_data_.reset();
        tree_value_map_.clear();
    }

    virtual bool isResign(const Node* selected_node) const
    {
        const Action& action = selected_node->getAction();
        float root_win_rate = tree_.getRootNode()->getNormalizedMean(tree_value_map_);
        float action_win_rate = selected_node->getNormalizedMean(tree_value_map_);
        return (root_win_rate < config::actor_resign_threshold && action_win_rate < config::actor_resign_threshold);
    }

    virtual Node* selectChildByMaxCount(const Node* node) const
    {
        assert(node && !node->isLeaf());
        float max_count = 0.0f;
        Node* selected = nullptr;
        Node* child = node->getFirstChild();
        for (int i = 0; i < node->getNumChildren(); ++i, ++child) {
            if (child->getCount() <= max_count) { continue; }
            max_count = child->getCount();
            selected = child;
        }
        assert(selected != nullptr);
        return selected;
    }

    virtual Node* selectChildBySoftmaxCount(const Node* node, float temperature = 1.0f, float value_threshold = 0.1f) const
    {
        assert(node && !node->isLeaf());
        Node* selected = nullptr;
        Node* child = node->getFirstChild();
        Node* best_child = selectChildByMaxCount(node);
        float best_mean = best_child->getNormalizedMean(tree_value_map_);
        float sum = 0.0f;
        for (int i = 0; i < node->getNumChildren(); ++i, ++child) {
            float count = std::pow(child->getCount(), 1 / temperature);
            float mean = child->getNormalizedMean(tree_value_map_);
            if (count == 0 || (mean < best_mean - value_threshold)) { continue; }
            sum += count;
            float rand = utils::Random::randReal(sum);
            if (selected == nullptr || rand < count) { selected = child; }
        }
        assert(selected != nullptr);
        return selected;
    }

    virtual std::string getSearchDistributionString() const
    {
        const Node* root = getRootNode();
        Node* child = root->getFirstChild();
        std::ostringstream oss;
        for (int i = 0; i < root->getNumChildren(); ++i, ++child) {
            if (child->getCount() == 0) { continue; }
            oss << (oss.str().empty() ? "" : ",")
                << child->getAction().getActionID() << ":" << child->getCount();
        }
        return oss.str();
    }

    virtual std::vector<Node*> select() { return selectFromNode(getRootNode()); }

    virtual std::vector<Node*> selectFromNode(Node* start_node)
    {
        assert(start_node);
        Node* node = start_node;
        std::vector<Node*> node_path{node};
        while (!node->isLeaf()) {
            node = selectChildByPUCTScore(node);
            node_path.push_back(node);
        }
        return node_path;
    }

    virtual void expand(Node* leaf_node, const std::vector<ActionCandidate>& action_candidates)
    {
        assert(leaf_node && action_candidates.size() > 0);
        Node* child = tree_.allocateNodes(action_candidates.size());
        leaf_node->setFirstChild(child);
        leaf_node->setNumChildren(action_candidates.size());
        for (const auto& candidate : action_candidates) {
            child->reset();
            child->setAction(candidate.action_);
            child->setPolicy(candidate.policy_);
            child->setPolicyLogit(candidate.policy_logit_);
            ++child;
        }
    }

    virtual void backup(const std::vector<Node*>& node_path, const float value, const float reward = 0.0f)
    {
        assert(node_path.size() > 0);
        float updated_value = value;
        node_path.back()->setValue(value);
        node_path.back()->setReward(reward);
        for (int i = static_cast<int>(node_path.size() - 1); i >= 0; --i) {
            Node* node = node_path[i];
            float old_mean = node->getMean();
            node->add(updated_value);
            updateTreeValueMap(old_mean, node->getMean());
            updated_value = node->getReward() + config::actor_mcts_reward_discount * updated_value;
        }
    }

    inline int getNumSimulation() const { return tree_.getRootNode()->getCount(); }
    inline bool reachMaximumSimulation() const { return (getNumSimulation() == config::actor_num_simulation + 1); }
    inline Tree& getTree() { return tree_; }
    inline const Tree& getTree() const { return tree_; }
    inline Node* getRootNode() { return tree_.getRootNode(); }
    inline const Node* getRootNode() const { return tree_.getRootNode(); }
    inline TreeExtraData& getTreeExtraData() { return tree_extra_data_; }
    inline const TreeExtraData& getTreeExtraData() const { return tree_extra_data_; }
    inline std::map<float, int>& getTreeValueMap() { return tree_value_map_; }
    inline const std::map<float, int>& getTreeValueMap() const { return tree_value_map_; }

protected:
    virtual Node* selectChildByPUCTScore(const Node* node) const
    {
        assert(node && !node->isLeaf());
        Node* selected = nullptr;
        Node* child = node->getFirstChild();
        int total_simulation = node->getCount();
        float init_q_value = calculateInitQValue(node);
        float best_score = -std::numeric_limits<float>::max();
        for (int i = 0; i < node->getNumChildren(); ++i, ++child) {
            float score = child->getNormalizedPUCTScore(total_simulation, tree_value_map_, init_q_value);
            if (score <= best_score) { continue; }
            best_score = score;
            selected = child;
        }
        assert(selected != nullptr);
        return selected;
    }

    virtual float calculateInitQValue(const Node* node) const
    {
        // init Q value = avg Q value of all visited children + one loss
        assert(node && !node->isLeaf());
        float sum_of_win = 0.0f, sum = 0.0f;
        Node* child = node->getFirstChild();
        for (int i = 0; i < node->getNumChildren(); ++i, ++child) {
            if (child->getCount() == 0) { continue; }
            sum_of_win += child->getNormalizedMean(tree_value_map_);
            sum += 1;
        }
        return (sum_of_win - 1) / (sum + 1);
    }

    virtual void updateTreeValueMap(float old_value, float new_value)
    {
        if (!config::actor_mcts_value_rescale) { return; }
        if (tree_value_map_.count(old_value)) {
            assert(tree_value_map_[old_value] > 0);
            --tree_value_map_[old_value];
            if (tree_value_map_[old_value] == 0) { tree_value_map_.erase(old_value); }
        }
        ++tree_value_map_[new_value];
    }

    Tree tree_;
    TreeExtraData tree_extra_data_;
    std::map<float, int> tree_value_map_;
};

typedef Tree<MCTSNode> MCTSTree;
typedef TreeExtraData<MCTSNodeExtraData> MCTSTreeExtraData;
typedef BaseMCTS<MCTSNode, MCTSTree, MCTSTreeExtraData> MCTS;

} // namespace minizero::actor