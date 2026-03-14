import os
import torch
from src.utils.data_generator import generate_synthetic_data
from src.utils.data_prep import load_and_scale
from src.models.autoencoder import HealthAutoencoder
from src.models.rl_agent import RLAgent
from src.training.trainer import HealthTrainer
from src.utils.evaluator import ModelEvaluator

def main():
    # 1. Ensure folders exist
    os.makedirs('data/raw', exist_ok=True)
    os.makedirs('models', exist_ok=True)

    # 2. Generate Synthetic Data
    print("Step 1: Generating Data...")
    generate_synthetic_data(num_records=500)

    # 3. Load and Prep
    print("Step 2: Preprocessing...")
    loader, scaled_raw = load_and_scale('data/raw/health_logs.csv')

    # 4. Train Autoencoder (Base Model)
    print("Step 3: Training Base Model (Learning 'Normal')...")
    ae_model = HealthAutoencoder(input_dim=7)
    trainer = HealthTrainer(ae_model)
    trainer.train_autoencoder(loader, epochs=30)
    torch.save(ae_model.state_dict(), 'models/base_autoencoder.pth')

    # 5. Evaluate Accuracy
    print("Step 4: Evaluating Model Accuracy...")
    evaluator = ModelEvaluator()
    stats, _ = evaluator.get_accuracy_report(ae_model, scaled_raw)
    print(f"Training Complete. Suggest Anomaly Threshold: {stats['threshold_suggested']:.4f}")

    # 6. Initialize RL Agent (RLRF Ready)
    rl_agent = RLAgent(state_dim=3)
    torch.save(rl_agent.state_dict(), 'models/rl_agent.pth')
    print("RL Agent Initialized and Saved.")

if __name__ == "__main__":
    main()