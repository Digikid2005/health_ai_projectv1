import torch
import os
from src.models.autoencoder import HealthAutoencoder

def export_standardized_header(model_path, header_path):
    os.makedirs(os.path.dirname(header_path), exist_ok=True)
    
    model = HealthAutoencoder(input_dim=7)
    model.load_state_dict(torch.load(model_path, map_location='cpu', weights_only=True))
    model.eval()
    
    # Extract weights in order
    params = list(model.state_dict().items())
    
    with open(header_path, 'w') as f:
        f.write("#ifndef MODEL_WEIGHTS_H\n#define MODEL_WEIGHTS_H\n\n")
        
        # We index them numerically so C++ names are always the same
        for i, (name, param) in enumerate(params):
            weights = param.detach().numpy().flatten()
            f.write(f"// Original name: {name}\n")
            f.write(f"const float weight_{i}[] = {{")
            f.write(", ".join([f"{w:.8f}f" for w in weights]))
            f.write("};\n\n")
            
        f.write("#endif")
    
    print(f"✅ Standardized weights exported to {header_path}")
    print(f"Total layers exported: {len(params)}")

if __name__ == "__main__":
    export_standardized_header('models/base_autoencoder.pth', 'esp32_health_ai/model_weights.h')