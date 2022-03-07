#pragma once

#include "network.h"
#include <mutex>

namespace minizero::network {

class AlphaZeroNetworkOutput : public NetworkOutput {
public:
    float value_;
    std::vector<float> policy_;

    AlphaZeroNetworkOutput(int policy_size)
    {
        value_ = 0.0f;
        policy_.resize(policy_size, 0.0f);
    }
};

class AlphaZeroNetwork : public Network {
public:
    AlphaZeroNetwork()
    {
        num_action_channels_ = action_size_ = -1;
        batch_size_ = 0;
    }

    void LoadModel(const std::string& nn_file_name, const int gpu_id)
    {
        Network::LoadModel(nn_file_name, gpu_id);

        std::vector<torch::jit::IValue> dummy;
        num_action_channels_ = network_.get_method("get_num_action_channels")(dummy).toInt();
        action_size_ = network_.get_method("get_action_size")(dummy).toInt();
        batch_size_ = 0;
    }

    std::string ToString() const
    {
        std::ostringstream oss;
        oss << Network::ToString();
        oss << "Number of action channels: " << num_action_channels_ << std::endl;
        oss << "Action size: " << action_size_ << std::endl;
        return oss.str();
    }

    int PushBack(std::vector<float> features)
    {
        assert(static_cast<int>(features.size()) == GetNumInputChannels() * GetInputChannelHeight() * GetInputChannelWidth());

        int index;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            index = batch_size_++;
            tensor_input_.resize(batch_size_);
        }
        tensor_input_[index] = torch::from_blob(features.data(), {1, GetNumInputChannels(), GetInputChannelHeight(), GetInputChannelWidth()}).clone().to(GetDevice());
        return index;
    }

    std::vector<std::shared_ptr<NetworkOutput>> Forward()
    {
        auto forward_result = network_.forward(std::vector<torch::jit::IValue>{torch::cat(tensor_input_)}).toGenericDict();
        auto policy_output = torch::softmax(forward_result.at("policy").toTensor(), 1).to(at::kCPU);
        auto value_output = forward_result.at("value").toTensor().to(at::kCPU);

        const int policy_size = GetActionSize();
        std::vector<std::shared_ptr<NetworkOutput>> network_outputs;
        for (int i = 0; i < batch_size_; ++i) {
            network_outputs.emplace_back(std::make_shared<AlphaZeroNetworkOutput>(policy_size));
            auto alphazero_network_output = std::static_pointer_cast<AlphaZeroNetworkOutput>(network_outputs.back());
            alphazero_network_output->value_ = value_output[i].item<float>();
            std::copy(policy_output.data_ptr<float>() + i * policy_size,
                      policy_output.data_ptr<float>() + (i + 1) * policy_size,
                      alphazero_network_output->policy_.begin());
        }

        batch_size_ = 0;
        tensor_input_.clear();
        return network_outputs;
    }

    inline int GetNumActionChannels() const { return num_action_channels_; }
    inline int GetActionSize() const { return action_size_; }
    inline int GetBatchSize() const { return batch_size_; }

private:
    int num_action_channels_;
    int action_size_;
    int batch_size_;
    std::mutex mutex_;
    std::vector<torch::Tensor> tensor_input_;
};

} // namespace minizero::network