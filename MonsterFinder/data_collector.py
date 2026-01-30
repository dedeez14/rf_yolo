"""
RF Online Data Collector
Tool untuk mengumpulkan training data YOLO

Cara pakai:
1. Jalankan script ini
2. Tekan F5 untuk capture screenshot
3. Gambar bounding box di sekitar monster (klik + drag)
4. Tekan S untuk save
5. Tekan ESC untuk keluar
"""

import cv2
import numpy as np
import time
import os
import win32gui
import win32ui
import win32con
import win32api
from datetime import datetime

class DataCollector:
    def __init__(self):
        self.hwnd = None
        self.width = 0
        self.height = 0
        self.x = 0
        self.y = 0
        
        # Current boxes
        self.boxes = []
        self.current_box = None
        self.drawing = False
        self.start_point = None
        
        # Dataset paths
        self.images_dir = os.path.join(os.path.dirname(__file__), "dataset", "images")
        self.labels_dir = os.path.join(os.path.dirname(__file__), "dataset", "labels")
        
        os.makedirs(self.images_dir, exist_ok=True)
        os.makedirs(self.labels_dir, exist_ok=True)
        
        # Current screenshot
        self.screenshot = None
        self.display = None
        
        # Classes
        self.classes = ["monster"]  # Tambah class lain jika perlu
        self.current_class = 0
    
    def find_window(self):
        result = [None]
        def cb(hwnd, _):
            if win32gui.IsWindowVisible(hwnd):
                if "rf online" in win32gui.GetWindowText(hwnd).lower():
                    result[0] = hwnd
            return True
        win32gui.EnumWindows(cb, None)
        self.hwnd = result[0]
        if self.hwnd:
            rect = win32gui.GetWindowRect(self.hwnd)
            self.x = rect[0]
            self.y = rect[1]
            self.width = rect[2] - rect[0]
            self.height = rect[3] - rect[1]
            return True
        return False
    
    def capture_screen(self):
        try:
            wDC = win32gui.GetWindowDC(self.hwnd)
            dcObj = win32ui.CreateDCFromHandle(wDC)
            cDC = dcObj.CreateCompatibleDC()
            bmp = win32ui.CreateBitmap()
            bmp.CreateCompatibleBitmap(dcObj, self.width, self.height)
            cDC.SelectObject(bmp)
            cDC.BitBlt((0, 0), (self.width, self.height), dcObj, (0, 0), win32con.SRCCOPY)
            bits = bmp.GetBitmapBits(True)
            img = np.frombuffer(bits, dtype=np.uint8).reshape(self.height, self.width, 4)
            result = img[:, :, :3].copy()
            dcObj.DeleteDC()
            cDC.DeleteDC()
            win32gui.ReleaseDC(self.hwnd, wDC)
            win32gui.DeleteObject(bmp.GetHandle())
            return result
        except:
            return None
    
    def mouse_callback(self, event, x, y, flags, param):
        if event == cv2.EVENT_LBUTTONDOWN:
            self.drawing = True
            self.start_point = (x, y)
        
        elif event == cv2.EVENT_MOUSEMOVE:
            if self.drawing:
                self.current_box = (self.start_point, (x, y))
        
        elif event == cv2.EVENT_LBUTTONUP:
            self.drawing = False
            if self.start_point:
                # Normalize box
                x1, y1 = self.start_point
                x2, y2 = x, y
                if x1 > x2:
                    x1, x2 = x2, x1
                if y1 > y2:
                    y1, y2 = y2, y1
                
                if abs(x2 - x1) > 10 and abs(y2 - y1) > 10:
                    self.boxes.append(((x1, y1), (x2, y2), self.current_class))
                    print(f"[BOX] Added: ({x1},{y1}) to ({x2},{y2}) - {self.classes[self.current_class]}")
                
                self.current_box = None
                self.start_point = None
    
    def draw_boxes(self):
        if self.screenshot is None:
            return
        
        self.display = self.screenshot.copy()
        
        # Draw saved boxes
        for box in self.boxes:
            (x1, y1), (x2, y2), cls = box
            color = (0, 255, 0)  # Green
            cv2.rectangle(self.display, (x1, y1), (x2, y2), color, 2)
            cv2.putText(self.display, self.classes[cls], (x1, y1-5), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)
        
        # Draw current box
        if self.current_box:
            (x1, y1), (x2, y2) = self.current_box
            cv2.rectangle(self.display, (x1, y1), (x2, y2), (0, 255, 255), 2)
        
        # Draw instructions
        cv2.putText(self.display, "F5=Capture | S=Save | C=Clear | R=Undo | ESC=Exit", 
                   (10, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        cv2.putText(self.display, f"Boxes: {len(self.boxes)} | Class: {self.classes[self.current_class]}", 
                   (10, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
    
    def save_data(self):
        if self.screenshot is None or len(self.boxes) == 0:
            print("[ERROR] No screenshot or boxes to save")
            return False
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        
        # Save image
        img_path = os.path.join(self.images_dir, f"{timestamp}.jpg")
        cv2.imwrite(img_path, self.screenshot)
        
        # Save labels (YOLO format)
        label_path = os.path.join(self.labels_dir, f"{timestamp}.txt")
        h, w = self.screenshot.shape[:2]
        
        with open(label_path, "w") as f:
            for box in self.boxes:
                (x1, y1), (x2, y2), cls = box
                
                # Convert to YOLO format (normalized center x, center y, width, height)
                cx = ((x1 + x2) / 2) / w
                cy = ((y1 + y2) / 2) / h
                bw = (x2 - x1) / w
                bh = (y2 - y1) / h
                
                f.write(f"{cls} {cx:.6f} {cy:.6f} {bw:.6f} {bh:.6f}\n")
        
        print(f"[SAVED] {img_path}")
        print(f"[SAVED] {label_path}")
        
        # Count total images
        total = len([f for f in os.listdir(self.images_dir) if f.endswith('.jpg')])
        print(f"[TOTAL] {total} images in dataset")
        
        return True
    
    def run(self):
        print("=" * 50)
        print("  RF ONLINE DATA COLLECTOR")
        print("  For YOLO Training")
        print("=" * 50)
        
        if not self.find_window():
            print("[ERROR] RF Online not found!")
            input("Press Enter...")
            return
        
        print(f"[OK] RF Online ({self.width}x{self.height})")
        print()
        print("[INSTRUCTIONS]")
        print("  F5  = Capture screenshot from RF")
        print("  Click + Drag = Draw bounding box")
        print("  S   = Save current data")
        print("  C   = Clear all boxes")
        print("  R   = Remove last box")
        print("  ESC = Exit")
        print()
        
        # Count existing images
        total = len([f for f in os.listdir(self.images_dir) if f.endswith('.jpg')])
        print(f"[DATASET] {total} images already collected")
        print()
        
        cv2.namedWindow("Data Collector")
        cv2.setMouseCallback("Data Collector", self.mouse_callback)
        
        # Initial capture
        self.screenshot = self.capture_screen()
        
        while True:
            if self.screenshot is not None:
                self.draw_boxes()
                cv2.imshow("Data Collector", self.display)
            
            key = cv2.waitKey(50) & 0xFF
            
            # ESC = Exit
            if key == 27:
                break
            
            # F5 = Capture (116)
            if key == 116:
                self.screenshot = self.capture_screen()
                self.boxes = []
                print("[CAPTURED] New screenshot")
            
            # S = Save
            if key == ord('s') or key == ord('S'):
                if self.save_data():
                    self.boxes = []
                    self.screenshot = self.capture_screen()
            
            # C = Clear
            if key == ord('c') or key == ord('C'):
                self.boxes = []
                print("[CLEAR] All boxes removed")
            
            # R = Remove last
            if key == ord('r') or key == ord('R'):
                if self.boxes:
                    self.boxes.pop()
                    print("[UNDO] Removed last box")
        
        cv2.destroyAllWindows()
        print("\n[DONE] Data collection finished")


if __name__ == "__main__":
    collector = DataCollector()
    collector.run()
