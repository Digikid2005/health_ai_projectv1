import torch
import torch.nn as nn

class RLAgent(nn.Module):
    def __init__(self, state_dim=3): # Inputs: MSE_Error, Mood, Fatigue
        super(RLAgent, self).__init__()
        self.network = nn.Sequential(
            nn.Linear(state_dim, 16),
            nn.ReLU(),
            nn.Linear(16, 1),
            nn.Sigmoid() # Probability of flagging as 'Warning'
        )

    def forward(self, x):
        return self.network(x)