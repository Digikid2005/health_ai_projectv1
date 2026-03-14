import pandas as pd
import numpy as np
from datetime import datetime, timedelta

def generate_synthetic_data(num_records=1000):
    np.random.seed(42)
    data = []
    start_time = datetime(2026, 3, 1)

    for i in range(num_records):
        timestamp = start_time + timedelta(hours=i*8)
        
        # Normal Ranges
        weight = np.random.normal(70, 2)
        bp_systolic = np.random.normal(120, 5)
        bp_diastolic = np.random.normal(80, 4)
        sugar = np.random.normal(90, 10)
        sleep = np.random.normal(7, 1)
        mood = np.random.choice([1, 2, 3, 4, 5]) # 1: Poor, 5: Great
        fatigue = np.random.choice([1, 2, 3, 4, 5]) # 1: Low, 5: High
        pregnant = "yes" if i % 10 == 0 else "no" # Every 10th user for testing

        # Injecting Anomalies (5% of data)
        if np.random.random() < 0.05:
            sugar += np.random.uniform(40, 100)  # Sugar Spike
            bp_systolic += np.random.uniform(30, 50) # BP Spike
            sleep -= 3 # Sleep Deprivation

        data.append([timestamp, weight, bp_systolic, bp_diastolic, sugar, sleep, mood, fatigue, pregnant])

    columns = ['Timestamp', 'Weight', 'BP_Systolic', 'BP_Diastolic', 'Sugar', 'Sleep_Duration', 'Mood', 'Fatigue', 'Pregnant']
    df = pd.DataFrame(data, columns=columns)
    df.to_csv('data/raw/health_logs.csv', index=False)
    print("Synthetic dataset generated: data/raw/health_logs.csv")

if __name__ == "__main__":
    generate_synthetic_data()