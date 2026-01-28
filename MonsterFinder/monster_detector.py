"""
Monster Detector - Lightweight Object Detection for RF Online Bot
Menggunakan OpenCV untuk deteksi monster yang lebih akurat
"""

import cv2
import numpy as np
import win32gui
import win32ui
import win32con
from typing import List, Tuple, Dict
import time

class LightweightMonsterDetector:
    """Detector monster ringan menggunakan OpenCV"""
    
    def __init__(self, window_name: str = "RF Online"):
        self.window_name = window_name
        self.hwnd = None
        self.window_rect = None
        self.templates = {}  # Template images untuk template matching
        self.last_frame = None
        
    def find_window(self) -> bool:
        """Cari window RF Online"""
        self.hwnd = win32gui.FindWindow(None, self.window_name)
        if self.hwnd == 0:
            return False
        
        self.window_rect = win32gui.GetWindowRect(self.hwnd)
        return True
    
    def capture_screen(self, x: int = 0, y: int = 0, width: int = 1400, height: int = 700) -> np.ndarray:
        """Capture screenshot dari game window"""
        if not self.hwnd:
            if not self.find_window():
                return None
        
        # Get window DC
        wDC = win32gui.GetWindowDC(self.hwnd)
        dcObj = win32ui.CreateDCFromHandle(wDC)
        cDC = dcObj.CreateCompatibleDC()
        
        # Create bitmap
        dataBitMap = win32ui.CreateBitmap()
        dataBitMap.CreateCompatibleBitmap(dcObj, width, height)
        cDC.SelectObject(dataBitMap)
        
        # Copy screen
        cDC.BitBlt((0, 0), (width, height), dcObj, (x, y), win32con.SRCCOPY)
        
        # Convert to numpy array
        signedIntsArray = dataBitMap.GetBitmapBits(True)
        img = np.frombuffer(signedIntsArray, dtype='uint8')
        img.shape = (height, width, 4)
        
        # Convert BGRA to RGB
        img = cv2.cvtColor(img, cv2.COLOR_BGRA2RGB)
        
        # Cleanup
        dcObj.DeleteDC()
        cDC.DeleteDC()
        win32gui.ReleaseDC(self.hwnd, wDC)
        win32gui.DeleteObject(dataBitMap.GetHandle())
        
        return img
    
    def detect_by_color(self, frame: np.ndarray, color_ranges: List[Dict]) -> List[Tuple]:
        """
        Deteksi monster berdasarkan warna (paling ringan)
        
        Args:
            frame: Frame gambar (BGR format)
            color_ranges: List of dict dengan format:
                {'name': 'MonsterName', 'lower': [B,G,R], 'upper': [B,G,R]}
        
        Returns:
            List of (x, y, w, h, name) bounding boxes
        """
        detections = []
        
        for color_range in color_ranges:
            name = color_range['name']
            lower = np.array(color_range['lower'], dtype=np.uint8)
            upper = np.array(color_range['upper'], dtype=np.uint8)
            
            # Create mask
            mask = cv2.inRange(frame, lower, upper)
            
            # Remove noise
            kernel = np.ones((3, 3), np.uint8)
            mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
            mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
            
            # Find contours
            contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            
            for contour in contours:
                area = cv2.contourArea(contour)
                if area < 20:  # Minimum area
                    continue
                
                x, y, w, h = cv2.boundingRect(contour)
                
                # Filter berdasarkan aspect ratio (monster biasanya tidak terlalu tipis/lebar)
                aspect_ratio = w / h if h > 0 else 0
                if aspect_ratio < 0.3 or aspect_ratio > 3.0:
                    continue
                
                detections.append((x, y, w, h, name))
        
        return detections
    
    def detect_by_template(self, frame: np.ndarray, templates: Dict[str, np.ndarray], 
                          threshold: float = 0.6) -> List[Tuple]:
        """
        Deteksi menggunakan template matching (ringan tapi perlu template)
        
        Args:
            frame: Frame gambar
            templates: Dict dengan key=name, value=template image (BGR)
            threshold: Similarity threshold (0-1)
        
        Returns:
            List of (x, y, w, h, name, confidence) bounding boxes
        """
        detections = []
        gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        for name, template in templates.items():
            if template is None:
                continue
            
            gray_template = cv2.cvtColor(template, cv2.COLOR_BGR2GRAY) if len(template.shape) == 3 else template
            
            # Template matching
            result = cv2.matchTemplate(gray_frame, gray_template, cv2.TM_CCOEFF_NORMED)
            locations = np.where(result >= threshold)
            
            h, w = gray_template.shape
            
            # Group nearby detections
            for pt in zip(*locations[::-1]):
                x, y = pt
                confidence = result[y, x]
                
                # Check if too close to existing detection
                too_close = False
                for det in detections:
                    dx = abs(det[0] - x)
                    dy = abs(det[1] - y)
                    if dx < w and dy < h:
                        if confidence > det[5]:  # Replace with higher confidence
                            detections.remove(det)
                        else:
                            too_close = True
                        break
                
                if not too_close:
                    detections.append((x, y, w, h, name, confidence))
        
        return detections
    
    def detect_by_motion(self, frame: np.ndarray, min_area: int = 50) -> List[Tuple]:
        """
        Deteksi berdasarkan pergerakan (ringan, tidak perlu template)
        
        Args:
            frame: Current frame
            min_area: Minimum area untuk dianggap sebagai monster
        
        Returns:
            List of (x, y, w, h) bounding boxes
        """
        detections = []
        
        if self.last_frame is None:
            self.last_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            return detections
        
        # Convert to grayscale
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        # Calculate difference
        diff = cv2.absdiff(self.last_frame, gray)
        
        # Threshold
        _, thresh = cv2.threshold(diff, 30, 255, cv2.THRESH_BINARY)
        
        # Remove noise
        kernel = np.ones((5, 5), np.uint8)
        thresh = cv2.morphologyEx(thresh, cv2.MORPH_OPEN, kernel)
        thresh = cv2.morphologyEx(thresh, cv2.MORPH_CLOSE, kernel)
        
        # Find contours
        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        for contour in contours:
            area = cv2.contourArea(contour)
            if area < min_area:
                continue
            
            x, y, w, h = cv2.boundingRect(contour)
            
            # Filter berdasarkan ukuran dan posisi
            if w < 10 or h < 10:
                continue
            
            # Skip jika terlalu dekat dengan center (player)
            center_x = frame.shape[1] // 2
            center_y = frame.shape[0] // 2
            dist = np.sqrt((x + w/2 - center_x)**2 + (y + h/2 - center_y)**2)
            
            if dist < 100:  # Skip player area
                continue
            
            detections.append((x, y, w, h))
        
        # Update last frame
        self.last_frame = gray
        
        return detections
    
    def detect_combined(self, frame: np.ndarray, color_ranges: List[Dict] = None,
                       use_motion: bool = True) -> List[Dict]:
        """
        Kombinasi deteksi warna + motion untuk hasil lebih akurat
        
        Returns:
            List of dict dengan format:
            {
                'x': int, 'y': int, 'w': int, 'h': int,
                'name': str, 'confidence': float, 'method': str
            }
        """
        all_detections = []
        
        # Color detection
        if color_ranges:
            color_detections = self.detect_by_color(frame, color_ranges)
            for x, y, w, h, name in color_detections:
                all_detections.append({
                    'x': x, 'y': y, 'w': w, 'h': h,
                    'name': name, 'confidence': 0.8, 'method': 'color'
                })
        
        # Motion detection
        if use_motion:
            motion_detections = self.detect_by_motion(frame)
            for x, y, w, h in motion_detections:
                # Check if overlaps with color detection
                overlaps = False
                for det in all_detections:
                    if self._boxes_overlap((x, y, w, h), 
                                          (det['x'], det['y'], det['w'], det['h'])):
                        det['confidence'] = min(1.0, det['confidence'] + 0.2)
                        overlaps = True
                        break
                
                if not overlaps:
                    all_detections.append({
                        'x': x, 'y': y, 'w': w, 'h': h,
                        'name': 'Unknown', 'confidence': 0.6, 'method': 'motion'
                    })
        
        return all_detections
    
    def _boxes_overlap(self, box1: Tuple, box2: Tuple) -> bool:
        """Check if two boxes overlap"""
        x1, y1, w1, h1 = box1
        x2, y2, w2, h2 = box2
        
        return not (x1 + w1 < x2 or x2 + w2 < x1 or y1 + h1 < y2 or y2 + h2 < y1)
    
    def draw_detections(self, frame: np.ndarray, detections: List[Dict]) -> np.ndarray:
        """Draw bounding boxes pada frame"""
        result = frame.copy()
        
        for det in detections:
            x, y, w, h = det['x'], det['y'], det['w'], det['h']
            name = det.get('name', 'Unknown')
            confidence = det.get('confidence', 0.0)
            method = det.get('method', 'unknown')
            
            # Color berdasarkan method
            if method == 'color':
                color = (0, 255, 0)  # Green
            elif method == 'motion':
                color = (255, 0, 0)  # Blue
            else:
                color = (0, 0, 255)  # Red
            
            # Draw rectangle
            cv2.rectangle(result, (x, y), (x + w, y + h), color, 2)
            
            # Draw label
            label = f"{name} {confidence:.2f}"
            cv2.putText(result, label, (x, y - 10), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)
        
        return result


def load_monster_colors_from_ini(ini_path: str = "monsters.ini") -> List[Dict]:
    """Load monster color ranges dari monsters.ini"""
    color_ranges = []
    
    try:
        with open(ini_path, 'r') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                
                parts = line.split(',')
                if len(parts) < 6:
                    continue
                
                name = parts[0].strip()
                r = int(parts[1].strip())
                g = int(parts[2].strip())
                b = int(parts[3].strip())
                tolerance = int(parts[4].strip())
                
                # Convert RGB to BGR untuk OpenCV
                # Create range dengan tolerance
                lower = [max(0, b - tolerance), max(0, g - tolerance), max(0, r - tolerance)]
                upper = [min(255, b + tolerance), min(255, g + tolerance), min(255, r + tolerance)]
                
                color_ranges.append({
                    'name': name,
                    'lower': lower,
                    'upper': upper
                })
    except FileNotFoundError:
        print(f"File {ini_path} not found. Using default colors.")
        # Default colors untuk RF Online
        color_ranges = [
            {'name': 'Bellato', 'lower': [50, 50, 150], 'upper': [150, 150, 255]},
            {'name': 'Accretian', 'lower': [50, 50, 150], 'upper': [150, 150, 255]},
            {'name': 'Cora', 'lower': [100, 50, 100], 'upper': [200, 150, 200]},
        ]
    
    return color_ranges


def main():
    """Test detector"""
    print("=== Lightweight Monster Detector ===")
    print("Initializing detector...")
    
    detector = LightweightMonsterDetector("RF Online")
    
    if not detector.find_window():
        print("ERROR: RF Online window not found!")
        print("Please start the game first.")
        return
    
    print("Window found!")
    print("Loading monster colors...")
    
    color_ranges = load_monster_colors_from_ini("monsters.ini")
    print(f"Loaded {len(color_ranges)} monster color profiles")
    
    print("\nStarting detection loop...")
    print("Press 'q' to quit, 's' to save screenshot")
    
    frame_count = 0
    fps_start = time.time()
    
    while True:
        # Capture screen
        frame = detector.capture_screen(width=1400, height=700)
        if frame is None:
            print("Failed to capture screen")
            break
        
        # Detect monsters
        detections = detector.detect_combined(frame, color_ranges, use_motion=True)
        
        # Draw detections
        result = detector.draw_detections(frame, detections)
        
        # Calculate FPS
        frame_count += 1
        if frame_count % 30 == 0:
            fps = 30 / (time.time() - fps_start)
            fps_start = time.time()
            print(f"FPS: {fps:.1f} | Detections: {len(detections)}")
        
        # Show result
        cv2.imshow("Monster Detector", cv2.cvtColor(result, cv2.COLOR_RGB2BGR))
        
        # Handle keys
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('s'):
            cv2.imwrite(f"detection_{int(time.time())}.png", 
                       cv2.cvtColor(result, cv2.COLOR_RGB2BGR))
            print("Screenshot saved!")
    
    cv2.destroyAllWindows()
    print("Detector stopped.")


if __name__ == "__main__":
    main()
