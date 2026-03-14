import numpy as np
import torch

class ModelEvaluator:
    @staticmethod
    def get_accuracy_report(model, scaled_data):
        model.eval()
        with torch.no_grad():
            inputs = torch.FloatTensor(scaled_data)
            outputs = model(inputs)
            # Accuracy in Autoencoders is measured by Reconstruction Error
            mse = torch.mean((inputs - outputs)**2, dim=1).numpy()
            
            report = {
                "avg_reconstruction_error": np.mean(mse),
                "max_error": np.max(mse),
                "threshold_suggested": np.mean(mse) + (2 * np.std(mse))
            }
        return report, mse