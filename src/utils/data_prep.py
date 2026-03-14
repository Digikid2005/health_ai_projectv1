import pandas as pd
import torch
from sklearn.preprocessing import StandardScaler
from torch.utils.data import DataLoader, TensorDataset
import joblib  # Fixed the name here

def load_and_scale(csv_path):
    df = pd.read_csv(csv_path)
    # Features used by the model
    features = ['Weight', 'BP_Systolic', 'BP_Diastolic', 'Sugar', 'Sleep_Duration', 'Mood', 'Fatigue']
    
    scaler = StandardScaler()
    scaled_data = scaler.fit_transform(df[features])
    
    # Use joblib to save the scaler correctly (better for sklearn objects)
    joblib.dump(scaler, 'models/scaler.joblib')
    
    dataset = TensorDataset(torch.FloatTensor(scaled_data))
    return DataLoader(dataset, batch_size=16, shuffle=True), scaled_data