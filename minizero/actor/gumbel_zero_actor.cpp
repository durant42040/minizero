#include "gumbel_zero_actor.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace minizero::actor {

using namespace minizero;
using namespace network;

void GumbelZeroActor::afterNNEvaluation(const std::shared_ptr<network::NetworkOutput>& network_output)
{
    ZeroActor::afterNNEvaluation(network_output);
    sequentialHalving();
}

std::string GumbelZeroActor::getMCTSPolicy() const
{
    // calculate value for non-visisted nodes
    float pi_sum = 0.0f, q_sum = 0.0f;
    for (int i = 0; i < getMCTS()->getRootNode()->getNumChildren(); ++i) {
        MCTSNode* child = getMCTS()->getRootNode()->getChild(i);
        if (child->getCount() == 0) { continue; }
        float value = (child->getAction().getPlayer() == env::Player::kPlayer1 ? child->getValue() : -child->getValue());
        pi_sum += child->getPolicy();
        q_sum += child->getPolicy() * value;
    }
    float value_pi = (getMCTS()->getRootNode()->getChild(0)->getAction().getPlayer() == env::Player::kPlayer1 ? getMCTS()->getRootNode()->getValue() : -getMCTS()->getRootNode()->getValue());
    float non_visited_node_value = 1.0 / (1 + config::actor_num_simulation) * (value_pi + (config::actor_num_simulation / pi_sum) * q_sum);

    // calculate completed Q-values
    std::unordered_map<int, float> new_logits;
    float max_logit = -std::numeric_limits<float>::max();
    for (int i = 0; i < getMCTS()->getRootNode()->getNumChildren(); ++i) {
        MCTSNode* child = getMCTS()->getRootNode()->getChild(i);
        float value = (child->getCount() == 0 ? non_visited_node_value : (child->getAction().getPlayer() == env::Player::kPlayer1 ? child->getValue() : -child->getValue()));
        float logit_without_noise = child->getPolicyLogit() - child->getPolicyNoise();
        float score = logit_without_noise + (config::actor_gumbel_sigma_visit_c + 1) * config::actor_gumbel_sigma_scale_c * value;
        new_logits.insert({child->getAction().getActionID(), score});
        max_logit = fmax(max_logit, score);
    }

    // return normalized completed Q-values
    std::ostringstream oss;
    for (auto& logit : new_logits) {
        logit.second = logit.second - max_logit;
        if (logit.second < -38)
            continue;
        oss << (oss.str().empty() ? "" : ",")
            << logit.first << ":" << exp(logit.second);
    }
    return oss.str();
}

MCTSNode* GumbelZeroActor::decideActionNode()
{
    if (config::actor_select_action_by_count) {
        assert(candidates_.size() > 0);
        sortCandidatesByScore();
        return candidates_[0];
    } else if (config::actor_select_action_by_softmax_count) {
        return getMCTS()->selectChildBySoftmaxCount(getMCTS()->getRootNode(), config::actor_select_action_softmax_temperature);
    }

    assert(false);
    return nullptr;
}

std::vector<MCTSNode*> GumbelZeroActor::selection()
{
    std::vector<MCTSNode*> node_path;
    if (getMCTS()->getNumSimulation() == 0) {
        node_path = getMCTS()->select();
    } else {
        assert(candidates_.size() > 0);
        sort(candidates_.begin(), candidates_.end(), [](const MCTSNode* lhs, const MCTSNode* rhs) {
            return (lhs->getCount() < rhs->getCount() || (lhs->getCount() == rhs->getCount() && lhs->getPolicyLogit() > rhs->getPolicyLogit()));
        });
        node_path = getMCTS()->selectFromNode(candidates_[0]);
        node_path.insert(node_path.begin(), getMCTS()->getRootNode());
    }
    return node_path;
}

void GumbelZeroActor::sequentialHalving()
{
    if (getMCTS()->getNumSimulation() == 1) {
        // collect candidates
        candidates_.clear();
        for (int i = 0; i < getMCTS()->getRootNode()->getNumChildren(); ++i) { candidates_.push_back(getMCTS()->getRootNode()->getChild(i)); }
        sort(candidates_.begin(), candidates_.end(), [](const MCTSNode* lhs, const MCTSNode* rhs) { return lhs->getPolicyLogit() > rhs->getPolicyLogit(); });
        if (static_cast<int>(candidates_.size()) > config::actor_gumbel_sample_size) { candidates_.resize(config::actor_gumbel_sample_size); }
        sample_size_ = config::actor_gumbel_sample_size;
        simulation_budget_ = std::max(1.0, std::floor(config::actor_num_simulation / (std::log2(config::actor_gumbel_sample_size) * sample_size_)));
    } else {
        bool all_candidates_reach_budget = true;
        for (auto node : candidates_) {
            if (node->getCount() >= simulation_budget_) { continue; }
            all_candidates_reach_budget = false;
            break;
        }

        if (all_candidates_reach_budget) {
            int next_budget = std::floor(config::actor_num_simulation / (std::log2(config::actor_gumbel_sample_size) * sample_size_ / 2));
            if (next_budget > 0 && sample_size_ > 2) {
                sample_size_ /= 2;
                assert(sample_size_ > 0);
                sortCandidatesByScore();
                if (static_cast<int>(candidates_.size()) > sample_size_) { candidates_.resize(sample_size_); }
                simulation_budget_ = candidates_[0]->getCount() + next_budget;
            }
        }
    }
}

void GumbelZeroActor::sortCandidatesByScore()
{
    assert(!candidates_.empty());
    sort(candidates_.begin(), candidates_.end(), [](const MCTSNode* lhs, const MCTSNode* rhs) {
        float min_value = -std::numeric_limits<float>::max();
        float lhs_value = (lhs->getAction().getPlayer() == env::Player::kPlayer1 ? lhs->getMean() : -lhs->getMean());
        float lhs_score = lhs->getPolicyLogit() + (config::actor_gumbel_sigma_visit_c + 1) * config::actor_gumbel_sigma_scale_c * lhs_value;
        lhs_score = (lhs->getCount() > 0 ? lhs_score : min_value);
        float rhs_value = (rhs->getAction().getPlayer() == env::Player::kPlayer1 ? rhs->getMean() : -rhs->getMean());
        float rhs_score = rhs->getPolicyLogit() + (config::actor_gumbel_sigma_visit_c + 1) * config::actor_gumbel_sigma_scale_c * rhs_value;
        rhs_score = (rhs->getCount() > 0 ? rhs_score : min_value);
        return lhs_score > rhs_score;
    });
}

} // namespace minizero::actor
