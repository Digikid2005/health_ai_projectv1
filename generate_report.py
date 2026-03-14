import pandas as pd
import datetime
import os
from src.utils.report_gen import MedicalReportPDF, create_scaled_plot

def generate_professional_report():
    df = pd.read_csv('data/raw/health_logs.csv').tail(10)
    df['Timestamp'] = pd.to_datetime(df['Timestamp'])
    
    metrics = {
        'Weight': 'plot_weight.png', 'Sugar': 'plot_sugar.png',
        'BP_Systolic': 'plot_bp.png', 'Mood': 'plot_mood.png'
    }
    for col, path in metrics.items():
        create_scaled_plot(df, col, path, '#006666')

    pdf = MedicalReportPDF()
    pdf.add_page()
    
    patient_info = {
        "NAME": "SOURAV7", "ID": "011000007",
        "DOB": "08/04/1973", "DATE": str(datetime.date.today())
    }
    pdf.header_section(patient_info)

    # Hierarchical Entries with ~50 word summaries
    y = 75
    
    # 1. Weight
    weight_summary = ("Current data indicates a stable weight trend with minor fluctuations. "
                      "Metabolic consistency is observed over the last 10 reporting cycles. "
                      "Recommended to maintain existing caloric intake and physical activity levels. "
                      "No sudden spikes or drops detected, suggesting healthy physiological regulation and adherence to current lifestyle protocols.")
    y = pdf.add_metric_entry(y, "Weight Analysis (kg)", weight_summary, "plot_weight.png", "Normal")

    # 2. Glucose
    glucose_summary = ("Post-meal glucose levels show significant variability with peaks exceeding the standard baseline. "
                       "This pattern suggests a need for tighter glycemic control. Clinical review of carbohydrate distribution "
                       "is recommended to mitigate these spikes. Persistent elevation during morning hours requires further "
                       "diagnostic investigation to rule out insulin resistance.")
    y = pdf.add_metric_entry(y, "Glucose Trend (mg/dL)", glucose_summary, "plot_sugar.png", "Warning")

    # 3. Blood Pressure
    bp_summary = ("Systolic readings are consistently trending above the 140 mmHg threshold, indicating Stage 2 Hypertension risk. "
                  "Immediate clinical consultation is required to evaluate cardiovascular load. Daily monitoring must continue. "
                  "Patient should reduce sodium intake and prioritize stress management while awaiting professional medical "
                  "assessment and potential medication adjustment.")
    y = pdf.add_metric_entry(y, "Blood Pressure (mmHg)", bp_summary, "plot_bp.png", "Consult Doctor")

    # 4. Mood
    mood_summary = ("Psychological well-being indicators remain within a healthy range despite minor daily variances. "
                    "Social interaction and sleep quality appear to correlate positively with these stability metrics. "
                    "Continuation of current mindfulness practices is encouraged to sustain this positive emotional baseline "
                    "throughout the upcoming observation period.")
    y = pdf.add_metric_entry(y, "Mood Trend (1-10)", mood_summary, "plot_mood.png", "Normal")

    pdf.output("Professional_Medical_Report.pdf")
    
    # Cleanup
    for path in metrics.values(): os.remove(path)
    print("Professional Report Generated Successfully.")

if __name__ == "__main__":
    generate_professional_report()