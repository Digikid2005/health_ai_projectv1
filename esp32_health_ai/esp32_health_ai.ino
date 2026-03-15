#include <Arduino.h>
#include <math.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "model_weights.h"

// --- Configuration ---
const char* ssid = "Health-AI-Scanner";
const char* password = "password123";
const int BUTTON_PIN = 0; // BOOT Button for Reinforcement Learning
bool is_pregnant = true;  // Maternal Health Toggle

WebServer server(80);
Preferences prefs;

// --- Global AI Variables ---
float anomaly_threshold;
float means[7] = {70.2, 122.5, 81.3, 95.8, 7.2, 3.1, 2.9}; 
float stds[7] = {4.8, 12.4, 8.2, 15.5, 1.2, 0.9, 0.8};

// --- Mock Data for Trends (Last 7 Readings) ---
float w_data[7] = {72.5, 71.8, 71.2, 70.5, 70.8, 69.5, 69.0};
float s_data[7] = {92.0, 110.0, 85.0, 125.0, 105.0, 98.0, 110.0};
float b_data[7] = {115.0, 125.0, 120.0, 145.0, 130.0, 135.0, 142.0};

// --- AI Inference Logic ---
float run_inference(float* raw_input) {
    float scaled[7], h1[16], reconstructed[7], mse = 0;
    for(int i=0; i<7; i++) scaled[i] = (raw_input[i] - means[i]) / stds[i];
    for (int i = 0; i < 16; i++) {
        h1[i] = weight_1[i];
        for (int j = 0; j < 7; j++) h1[i] += scaled[j] * weight_0[i * 7 + j];
        if (h1[i] < 0) h1[i] = 0; // ReLU
    }
    for (int i = 0; i < 7; i++) {
        reconstructed[i] = weight_9[i]; 
        for (int j = 0; j < 16; j++) reconstructed[i] += h1[j] * weight_8[i * 16 + j];
        mse += pow(scaled[i] - reconstructed[i], 2);
    }
    return mse / 7.0;
}

// --- PDF Vector Graphics Engine ---
void draw_dashed_grid(File &f, float x, float y, float w, float h) {
    f.print("q 0.2 w 0.8 0.8 0.8 RG [2 2] 0 d "); 
    for(int i=0; i<=4; i++) {
        float hy = y + (i * h / 4);
        f.print(String(x) + " " + String(hy) + " m " + String(x+w) + " " + String(hy) + " l S ");
        float vx = x + (i * w / 4);
        f.print(String(vx) + " " + String(y) + " m " + String(vx) + " " + String(y+h) + " l S ");
    }
    f.print("Q\n");
}

void draw_trend_line(File &f, float x, float y, float w, float h, float* data, float min_v, float max_v) {
    f.print("q 1.2 w 0 0.4 0.4 RG ");
    for (int i = 0; i < 7; i++) {
        float px = x + (float)i * (w / 6.0);
        float py = y + (data[i] - min_v) * (h / (max_v - min_v));
        if (i == 0) f.print(String(px) + " " + String(py) + " m ");
        else f.print(String(px) + " " + String(py) + " l ");
    }
    f.print("S Q\n");
    for (int i = 0; i < 7; i++) {
        float px = x + (float)i * (w / 6.0);
        float py = y + (data[i] - min_v) * (h / (max_v - min_v));
        f.print("q 0 0.4 0.4 rg " + String(px) + " " + String(py) + " 2 0 360 arc f Q\n");
    }
}

void draw_dashboard_row(File &f, int y, String title, String summary, String status, float* data, float min_v, float max_v) {
    // Header & Divider
    f.print("BT /F1 11 Tf 0 0.4 0.4 rg 30 " + String(y + 110) + " Td (" + title + ") Tj ET\n");
    f.print("q 0.5 w 0 0.4 0.4 RG 30 " + String(y + 105) + " m 565 " + String(y + 105) + " l S Q\n");
    
    // Graph
    draw_dashed_grid(f, 40, y + 15, 200, 80);
    draw_trend_line(f, 40, y + 15, 200, 80, data, min_v, max_v);
    
    // Summary Text
    f.print("BT /F2 9 Tf 0 0 0 rg 260 " + String(y + 85) + " Td (" + summary.substring(0, 50) + ") Tj 0 -12 Td (" + summary.substring(50, 100) + ") Tj 0 -12 Td (" + summary.substring(100) + ") Tj ET\n");

    // Status Box
    String color = (status == "NORMAL") ? "0 0.5 0" : (status == "WARNING") ? "1 0.6 0" : "0.8 0 0";
    f.print("q 0.8 w " + color + " RG 485 " + String(y + 35) + " 80 45 re S Q\n");
    f.print("BT /F1 8 Tf 0.4 0.4 0.4 rg 508 " + String(y + 70) + " Td (STATUS) Tj ET\n");
    f.print("BT /F1 9 Tf " + color + " rg 500 " + String(y + 50) + " Td (" + status + ") Tj ET\n");
}

void generate_report(const char* overall_status, float sugar, float bp) {
    if (!SPIFFS.begin(true)) return;
    File file = SPIFFS.open("/report.pdf", "w");
    
    file.print("%PDF-1.4\n1 0 obj << /Type /Catalog /Pages 2 0 R >> endobj\n2 0 obj << /Type /Pages /Kids [3 0 R] /Count 1 >> endobj\n");
    file.print("3 0 obj << /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Contents 4 0 R /Resources << /Font << /F1 << /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >> /F2 << /Type /Font /Subtype /Type1 /BaseFont /Helvetica >> >> >> >> endobj\n");
    file.print("4 0 obj << /Length 12000 >> stream\nq\n");

    // Static Dashboard UI
    file.print("0 0.4 0.4 rg 0 760 595 82 re f\n0.94 0.96 0.96 rg 30 680 535 60 re f Q\n");
    file.print("BT /F1 18 Tf 1 1 1 rg 50 800 Td (COMPREHENSIVE PATIENT MEDICAL REPORT) Tj ET\n");
    file.print("BT /F1 9 Tf 0 0 0 rg 50 715 Td (PATIENT NAME: SOURAV7) Tj 220 0 Td (DATE OF BIRTH: 08/04/1973) Tj ET\n");
    file.print("BT /F1 9 Tf 50 695 Td (PATIENT ID: 011000007) Tj 220 0 Td (REPORT DATE: 2026-03-14) Tj ET\n");

    // Data Rows
    draw_dashboard_row(file, 530, "WEIGHT ANALYSIS (KG)", "Weight stability indicates metabolic consistency over last cycles. Maintain activity.", "NORMAL", w_data, 65, 75);
    draw_dashboard_row(file, 380, "GLUCOSE TREND (MG/DL)", "Glycemic variability detected. Post-meal peaks exceed baseline. Review diet distribution.", "WARNING", s_data, 70, 140);
    
    if (is_pregnant) {
        draw_dashboard_row(file, 230, "MATERNAL HEALTH STATUS", "Gestational markers within stable thresholds. Systolic load monitored for maternal safety.", overall_status, b_data, 110, 150);
    } else {
        draw_dashboard_row(file, 230, "BLOOD PRESSURE (MMHG)", "Systolic trends are being monitored for hyper-variability and cardiovascular load.", overall_status, b_data, 110, 150);
    }

    file.print("endstream\nendobj\ntrailer << /Root 1 0 R /Size 4 >>\n%%EOF");
    file.close();
    Serial.println("✅ High-Fidelity PDF Generated.");
}

// --- Web Server Handlers ---
void handleRoot() {
    String h = "<html><body style='font-family:sans-serif;text-align:center;padding-top:10%;background:#f0f4f4;'>";
    h += "<div style='background:white;display:inline-block;padding:40px;border-radius:20px;box-shadow:0 10px 20px rgba(0,0,0,0.1);'>";
    h += "<h1 style='color:#006666;'>AI Health Portal</h1><p>Your professional report is ready for download.</p><br>";
    h += "<a href='/download' style='background:#006666;color:white;padding:15px 30px;text-decoration:none;border-radius:10px;font-weight:bold;'>DOWNLOAD PDF</a>";
    h += "</div></body></html>";
    server.send(200, "text/html", h);
}

void handleDownload() {
    File f = SPIFFS.open("/report.pdf", "r");
    server.sendHeader("Content-Type", "application/pdf");
    server.sendHeader("Content-Disposition", "attachment; filename=MedicalReport.pdf");
    server.sendHeader("Content-Length", String(f.size()));
    server.streamFile(f, "application/pdf");
    f.close();
}

// --- Setup & Loop ---
void setup() {
    Serial.begin(115200);
    unsigned long s = millis(); while (!Serial && millis() - s < 5000);
    SPIFFS.begin(true);
    prefs.begin("health-ai", false);
    anomaly_threshold = prefs.getFloat("threshold", 1.30);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    float mock_in[7] = {72.0, 145.0, 92.0, 110.0, 6.5, 4.0, 2.0};
    float mse = run_inference(mock_in);
    const char* st = (mse > anomaly_threshold) ? "CONSULT DOCTOR" : "NORMAL";
    
    generate_report(st, mock_in[3], mock_in[1]);

    WiFi.softAP(ssid, password);
    server.on("/", handleRoot);
    server.on("/download", handleDownload);
    server.begin();
    Serial.print("Access Portal at: "); Serial.println(WiFi.softAPIP());
}

void loop() {
    server.handleClient();
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50); anomaly_threshold += 0.15; prefs.putFloat("threshold", anomaly_threshold);
        Serial.print("RL Training: Threshold increased to "); Serial.println(anomaly_threshold);
        while(digitalRead(BUTTON_PIN) == LOW);
    }
}