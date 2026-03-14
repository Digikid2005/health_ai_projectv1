#include <Arduino.h>
#include <math.h>
#include "SPIFFS.h"
#include <Preferences.h> 
#include "model_weights.h" 

// --- Constants ---
const int INPUT_DIM = 7;
const int HIDDEN_1 = 16;
const int BUTTON_PIN = 0; // BOOT button on ESP32-S3

// --- Scaling Parameters (Derived from your dataset) ---
// [Weight, BP_Sys, BP_Dia, Sugar, Sleep, Mood, Fatigue]
float means[7] = {70.0, 120.0, 80.0, 100.0, 7.0, 3.0, 3.0}; 
float stds[7] = {5.0, 15.0, 10.0, 20.0, 1.5, 1.0, 1.0};

// --- Globals ---
Preferences prefs;
float anomaly_threshold;

// --- AI Inference Engine ---
float run_inference(float* raw_input) {
    float scaled_input[7];
    float h1[HIDDEN_1];
    float reconstructed[7];
    float mse = 0;

    // 1. Z-Score Normalization
    for(int i=0; i<7; i++) {
        scaled_input[i] = (raw_input[i] - means[i]) / stds[i];
    }

    // 2. Encoder Layer (Standardized weight_0 and weight_1)
    for (int i = 0; i < HIDDEN_1; i++) {
        h1[i] = weight_1[i]; // weight_1 is Bias
        for (int j = 0; j < INPUT_DIM; j++) {
            h1[i] += scaled_input[j] * weight_0[i * INPUT_DIM + j]; // weight_0 is Weight
        }
        if (h1[i] < 0) h1[i] = 0; // ReLU
    }

    // 3. Decoder Layer (Standardized weight_8 and weight_9)
    for (int i = 0; i < INPUT_DIM; i++) {
        reconstructed[i] = weight_9[i]; 
        for (int j = 0; j < HIDDEN_1; j++) {
            reconstructed[i] += h1[j] * weight_8[i * HIDDEN_1 + j];
        }
    }

    // 4. Calculate Normalized MSE
    for (int i = 0; i < INPUT_DIM; i++) {
        mse += pow(scaled_input[i] - reconstructed[i], 2);
    }
    return mse / INPUT_DIM;
}

// --- PDF Generation ---
void generate_pdf(const char* status, float sugar, float bp) {
    if (!SPIFFS.begin(true)) return;
    File file = SPIFFS.open("/health_report.pdf", "w");
    if (!file) return;

    file.println("%PDF-1.1");
    file.println("1 0 obj << /Type /Catalog /Pages 2 0 R >> endobj");
    file.println("2 0 obj << /Type /Pages /Kids [3 0 R] /Count 1 >> endobj");
    file.println("3 0 obj << /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Contents 4 0 R /Resources << /Font << /F1 << /Type /Font /Subtype /Type1 /BaseFont /Helvetica >> >> >> >> endobj");
    
    String text = "BT /F1 12 Tf 50 750 Td (ESP32-S3 AI HEALTH REPORT) Tj "
                  "0 -20 Td (Sugar: " + String(sugar) + " mg/dL) Tj "
                  "0 -20 Td (BP: " + String(bp) + " mmHg) Tj "
                  "0 -40 Td (STATUS: " + String(status) + ") Tj ET";
    
    file.println("4 0 obj << /Length " + String(text.length()) + " >> stream");
    file.println(text);
    file.println("endstream\nendobj");
    file.println("%%EOF");
    file.close();
    Serial.println("✅ PDF Report Stored in SPIFFS.");
}

void setup() {
    Serial.begin(115200);
    
    // Fix for ESP32-S3 Serial Handshake
    unsigned long start = millis();
    while (!Serial && (millis() - start < 5000)); 
    delay(1000);

    Serial.println("\n--- AI Health Engine Wake Up ---");
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // Load RL Threshold
    prefs.begin("health-ai", false);
    anomaly_threshold = prefs.getFloat("threshold", 0.85);
    Serial.print("Current Anomaly Threshold: "); Serial.println(anomaly_threshold, 4);

    // Mock User Data: [Weight, BPS, BPD, Sugar, Sleep, Mood, Fatigue]
    float current_vitals[7] = {72.0, 145.0, 92.0, 110.0, 6.5, 4.0, 2.0};
    
    float mse = run_inference(current_vitals);
    Serial.print("Normalized MSE Score: "); Serial.println(mse, 4);

    const char* status = (mse > anomaly_threshold) ? "WARNING" : "NORMAL";
    Serial.print("Prediction: "); Serial.println(status);

    generate_pdf(status, current_vitals[3], current_vitals[1]);
    Serial.println("Ready for User Feedback (Press BOOT button to train)...");
}

void loop() {
    // REINFORCEMENT LEARNING: ADAPT TO USER FEEDBACK
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50); // Debounce
        Serial.println("\n[RL Feedback] User disagrees with alert. Learning...");
        anomaly_threshold += 0.10; // Making model less sensitive
        prefs.putFloat("threshold", anomaly_threshold);
        Serial.print("New AI Threshold Saved: "); Serial.println(anomaly_threshold, 4);
        while(digitalRead(BUTTON_PIN) == LOW); // Wait for release
    }
}