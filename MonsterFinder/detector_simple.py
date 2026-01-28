"""
Ultra-Lightweight Monster Detector
Versi paling ringan - hanya menggunakan color detection
"""

import cv2
import numpy as np
import win32gui
import win32ui
import win32con
from typing import List, Tuple

class SimpleMonsterDetector:
    """Detector paling ringan - hanya color matching"""
    
    def __init__(self, window_name: str = "RF Online"):
        self.window_name = window_name
        self.hwnd = None
        
    def find_window(self) -> bool:
        """Cari window RF Online"""
        self.hwnd = win32gui.FindWindow(None, self.window_name)
        return self.hwnd != 0
    
    def capture_screen(self, x: int = 0, y: int = 0, width: int = 1400, height: int = 700) -> np.ndarray:
        """Capture screenshot - versi ringan"""
        if not self.hwnd:
            if not self.find_window():
                return None
        
        wDC = win32gui.GetWindowDC(self.hwnd)
        dcObj = win32ui.CreateDCFromHandle(wDC)
        cDC = dcObj.CreateCompatibleDC()
        dataBitMap = win32ui.CreateBitmap()
        dataBitMap.CreateCompatibleBitmap(dcObj, width, height)
        cDC.SelectObject(dataBitMap)
        cDC.BitBlt((0, 0), (width, height), dcObj, (x, y), win32con.SRCCOPY)
        
        signedIntsArray = dataBitMap.GetBitmapBits(True)
        img = np.frombuffer(signedIntsArray, dtype='uint8')
        img.shape = (height, width, 4)
        img = cv2.cvtColor(img, cv2.COLOR_BGRA2RGB)
        
        dcObj.DeleteDC()
        cDC.DeleteDC()
        win32gui.ReleaseDC(self.hwnd, wDC)
        win32gui.DeleteObject(dataBitMap.GetHandle())
        
        return img
    
    def detect_monsters(self, frame: np.ndarray, colors: List[dict]) -> List[Tuple]:
        """
        Deteksi monster berdasarkan warna
        
        Args:
            frame: Frame gambar (RGB)
            colors: List of {'name': str, 'r': int, 'g': int, 'b': int, 'tolerance': int}
        
        Returns:
            List of (x, y, w, h, name) bounding boxes
        """
        detections = []
        
        # Convert RGB to BGR untuk OpenCV
        frame_bgr = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
        
        for color_info in colors:
            name = color_info['name']
            r, g, b = color_info['r'], color_info['g'], color_info['b']
            tolerance = color_info.get('tolerance', 30)
            
            # Create color range (BGR format)
            lower = np.array([max(0, b - tolerance), max(0, g - tolerance), max(0, r - tolerance)], dtype=np.uint8)
            upper = np.array([min(255, b + tolerance), min(255, g + tolerance), min(255, r + tolerance)], dtype=np.uint8)
            
            # Create mask
            mask = cv2.inRange(frame_bgr, lower, upper)
            
            # Quick noise removal
            mask = cv2.medianBlur(mask, 3)
            
            # Find contours
            contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            
            for contour in contours:
                area = cv2.contourArea(contour)
                if area < 15:  # Minimum area
                    continue
                
                x, y, w, h = cv2.boundingRect(contour)
                
                # Filter berdasarkan ukuran
                if w < 5 or h < 5 or w > 200 or h > 200:
                    continue
                
                # Filter berdasarkan aspect ratio
                if w / h < 0.2 or w / h > 5.0:
                    continue
                
                # Skip jika terlalu dekat dengan center (player)
                center_x, center_y = frame.shape[1] // 2, frame.shape[0] // 2
                box_center_x, box_center_y = x + w // 2, y + h // 2
                dist = np.sqrt((box_center_x - center_x)**2 + (box_center_y - center_y)**2)
                
                if dist < 80:  # Skip player area
                    continue
                
                detections.append((x, y, w, h, name))
        
        return detections
    
    def draw_boxes(self, frame: np.ndarray, detections: List[Tuple]) -> np.ndarray:
        """Draw bounding boxes"""
        result = frame.copy()
        
        for x, y, w, h, name in detections:
            # Draw rectangle (RGB format)
            cv2.rectangle(result, (x, y), (x + w, y + h), (0, 255, 0), 2)
            cv2.putText(result, name, (x, y - 5), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
        
        return result


def load_colors_simple(ini_path: str = "monsters.ini") -> List[dict]:
    """Load colors dari monsters.ini"""
    colors = []
    
    try:
        with open(ini_path, 'r') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                
                parts = [p.strip() for p in line.split(',')]
                if len(parts) >= 5:
                    colors.append({
                        'name': parts[0],
                        'r': int(parts[1]),
                        'g': int(parts[2]),
                        'b': int(parts[3]),
                        'tolerance': int(parts[4])
                    })
    except:
        pass
    
    return colors


def main():
    """Test simple detector"""
    print("=== Simple Monster Detector ===")
    
    detector = SimpleMonsterDetector("RF Online")
    
    if not detector.find_window():
        print("RF Online window not found!")
        return
    
    colors = load_colors_simple("monsters.ini")
    print(f"Loaded {len(colors)} colors")
    
    print("Press 'q' to quit")
    
    while True:
        frame = detector.capture_screen()
        if frame is None:
            break
        
        detections = detector.detect_monsters(frame, colors)
        result = detector.draw_boxes(frame, detections)
        
        # Show
        cv2.imshow("Simple Detector", cv2.cvtColor(result, cv2.COLOR_RGB2BGR))
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
