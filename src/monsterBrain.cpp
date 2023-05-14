#include "monsterBrain.h"

monsterBrain::monsterBrain()
{
    //ctor
}

monsterBrain::~monsterBrain()
{
    //dtor
}
#include <torch/torch.h>

class ActorCriticNetwork : public torch::nn::Module {
public:
  ActorCriticNetwork(int input_size, int hidden_size, int num_actions)
      : actor(torch::nn::Sequential(torch::nn::Linear(input_size, hidden_size),
                                     torch::nn::ReLU(),
                                     torch::nn::Linear(hidden_size, num_actions),
                                     torch::nn::Softmax())),
        critic(torch::nn::Sequential(torch::nn::Linear(input_size, hidden_size),
                                      torch::nn::
#include <torch/torch.h>

class ActorCriticNetwork : public torch::nn::Module {
public:
  ActorCriticNetwork(int input_size, int hidden_size, int num_actions)
      : actor(torch::nn::Sequential(torch::nn::Linear(input_size, hidden_size),
                                     torch::nn::ReLU(),
                                     torch::nn::Linear(hidden_size, num_actions),
                                     torch::nn::Softmax())),
        critic(torch::nn::Sequential(torch::nn::Linear(input_size, hidden_size),
                                      torch::nn::Softmax()))
    }
