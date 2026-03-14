import torch
import torch.nn as nn
import torch.optim as optim

class HealthTrainer:
    def __init__(self, model, lr=0.001):
        self.model = model
        self.optimizer = optim.Adam(model.parameters(), lr=lr)
        self.criterion = nn.MSELoss()

    def train_autoencoder(self, loader, epochs=50):
        self.model.train()
        for epoch in range(epochs):
            total_loss = 0
            for batch in loader:
                data = batch[0]
                self.optimizer.zero_grad()
                output = self.model(data)
                loss = self.criterion(output, data)
                loss.backward()
                self.optimizer.step()
                total_loss += loss.item()
            if epoch % 10 == 0:
                print(f"Epoch {epoch} | Loss: {total_loss/len(loader):.4f}")

    def train_rl_step(self, agent, optimizer, state, action_taken, reward):
        # Reinforcement Learning Reward Function (RLRF)
        # action_taken: prob from agent, reward: +1 or -1
        log_prob = torch.log(action_taken)
        loss = -(log_prob * reward)
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        return loss.item()