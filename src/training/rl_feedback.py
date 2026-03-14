import torch

class RLRewardManager:
    def __init__(self, agent, optimizer):
        self.agent = agent
        self.optimizer = optimizer

    def update_policy(self, state_tensor, user_action, model_prediction):
        """
        user_action: 1 (User agrees with anomaly), -1 (User disagrees/False Alarm)
        model_prediction: The probability score the RL Agent gave
        """
        # Reward Function: Higher reward if user agrees with alert
        reward = torch.tensor([float(user_action)])
        
        # Policy Gradient Loss
        loss = -torch.log(model_prediction) * reward
        
        self.optimizer.zero_grad()
        loss.backward()
        self.optimizer.step()
        
        return loss.item()