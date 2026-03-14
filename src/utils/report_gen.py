import matplotlib.pyplot as plt
from fpdf import FPDF
import pandas as pd
import os

class MedicalReportPDF(FPDF):
    def __init__(self):
        super().__init__(orientation='P', unit='mm', format='A4')
        self.primary_teal = (0, 102, 102)
        self.bg_color = (252, 252, 252)

    def header_section(self, patient_info):
        self.set_fill_color(*self.primary_teal)
        self.rect(0, 0, 210, 40, 'F')
        self.set_text_color(255, 255, 255)
        self.set_font("Arial", 'B', 16)
        self.set_xy(10, 15)
        self.cell(0, 10, "COMPREHENSIVE PATIENT MEDICAL REPORT", 0, 1)
        
        self.set_fill_color(240, 245, 245)
        self.rect(10, 45, 190, 25, 'F')
        self.set_text_color(50, 50, 50)
        self.set_font("Arial", 'B', 9)
        
        self.set_xy(15, 48)
        self.cell(30, 5, "PATIENT NAME:", 0, 0); self.set_font("Arial", '', 9); self.cell(60, 5, patient_info["NAME"], 0, 0)
        self.set_font("Arial", 'B', 9); self.cell(30, 5, "DATE OF BIRTH:", 0, 0); self.set_font("Arial", '', 9); self.cell(60, 5, patient_info["DOB"], 0, 1)
        self.set_xy(15, 56)
        self.set_font("Arial", 'B', 9); self.cell(30, 5, "PATIENT ID:", 0, 0); self.set_font("Arial", '', 9); self.cell(60, 5, patient_info["ID"], 0, 0)
        self.set_font("Arial", 'B', 9); self.cell(30, 5, "REPORT DATE:", 0, 0); self.set_font("Arial", '', 9); self.cell(60, 5, patient_info["DATE"], 0, 1)

    def add_metric_entry(self, y_pos, title, summary, graph_path, status):
        # --- FIX: Page Break Logic ---
        # If remaining space is less than 65mm, start a new page
        if y_pos > 230: 
            self.add_page()
            y_pos = 20 # Start below the top margin on new page
        
        self.set_xy(10, y_pos)
        self.set_font("Arial", 'B', 11)
        self.set_text_color(*self.primary_teal)
        self.cell(0, 10, title.upper(), "B", 1)
        
        if os.path.exists(graph_path):
            self.image(graph_path, 10, y_pos + 12, w=85)
        
        self.set_xy(100, y_pos + 12)
        self.set_font("Arial", '', 9)
        self.set_text_color(60, 60, 60)
        self.multi_cell(70, 4.5, summary)
        
        status_x = 175
        status_y = y_pos + 12
        if "CONSULT" in status.upper(): color = (200, 0, 0); font_size = 7
        elif "WARNING" in status.upper(): color = (255, 140, 0); font_size = 8
        else: color = (0, 120, 0); font_size = 8

        self.set_draw_color(*color)
        self.set_line_width(0.5)
        self.rect(status_x, status_y, 25, 15)
        
        self.set_xy(status_x, status_y + 2)
        self.set_font("Arial", 'B', 7)
        self.set_text_color(100, 100, 100)
        self.cell(25, 4, "STATUS", 0, 1, 'C')
        
        self.set_xy(status_x, status_y + 7)
        self.set_text_color(*color)
        self.set_font("Arial", 'B', font_size)
        self.multi_cell(25, 4, status.upper(), 0, 'C')
        
        return y_pos + 60 

def create_scaled_plot(df, column, filename, color):
    plt.figure(figsize=(5, 2.5))
    plt.plot(df['Timestamp'], df[column], color=color, linewidth=1.5, marker='o', markersize=3)
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.xticks(rotation=20, fontsize=7)
    plt.yticks(fontsize=7)
    plt.title(f"{column} Analysis", fontsize=9, fontweight='bold')
    plt.tight_layout()
    plt.savefig(filename, dpi=150)
    plt.close()