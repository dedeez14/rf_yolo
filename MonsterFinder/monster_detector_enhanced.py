"""
RF Online Auto Bot v16.0
YOLO Object Detection
- Menggunakan trained YOLO model untuk deteksi monster
- Atau fallback ke hover scan jika model tidak ada
"""

import cv2
import numpy as np
import time
import os
import math
import win32gui
import win32ui
import win32con
import win32api
import threading

try:
    import pyautogui
    pyautogui.FAILSAFE = False
    pyautogui.PAUSE = 0.005
except:
    os.system("pip install pyautogui")
    import pyautogui
    pyautogui.FAILSAFE = False
    pyautogui.PAUSE = 0.005

# Try to load YOLO
YOLO_AVAILABLE = False
try:
    from ultralytics import YOLO
    YOLO_AVAILABLE = True
except Exception as e:
    print(f"[WARNING] YOLO Check Failed: {e}")
    print("Install with: pip install ultralytics")

STOP_FLAG = False


def keyboard_listener():
    global STOP_FLAG
    while not STOP_FLAG:
        if win32api.GetAsyncKeyState(0x1B) & 0x8000:
            STOP_FLAG = True
            print("\n[ESC] Stopping...")
            break
        time.sleep(0.1)


class RFBot:
    def __init__(self):
        self.hwnd = None
        self.width = 0
        self.height = 0
        self.x = 0
        self.y = 0
        self.kills = 0
        
        # YOLO model
        self.model = None
        self.use_yolo = False
        self.confidence = 0.25  # Lower threshold for testing
    
    def load_yolo_model(self):
        if not YOLO_AVAILABLE:
            return False
        
        # Priority 1: VPS/Training Output (models/monster_detector/weights/best.pt)
        vps_model = os.path.join(os.path.dirname(__file__), "models", "monster_detector", "weights", "best.pt")
        # Priority 2: Manual Copy (models/monster_detector.pt)
        local_model = os.path.join(os.path.dirname(__file__), "models", "monster_detector.pt")
        
        if os.path.exists(vps_model):
            model_path = vps_model
            print(f"[YOLO] Loading VPS model: {model_path}")
        elif os.path.exists(local_model):
            model_path = local_model
            print(f"[YOLO] Loading Local model: {model_path}")
        else:
            return False

        try:
            self.model = YOLO(model_path)
            self.use_yolo = True
            print("[YOLO] Model loaded successfully!")
            return True
        except Exception as e:
            print(f"[YOLO] Failed to load model: {e}")
            return False
    
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
    
    def capture_region(self, wx, wy, w, h):
        try:
            wDC = win32gui.GetWindowDC(self.hwnd)
            dcObj = win32ui.CreateDCFromHandle(wDC)
            cDC = dcObj.CreateCompatibleDC()
            bmp = win32ui.CreateBitmap()
            bmp.CreateCompatibleBitmap(dcObj, w, h)
            cDC.SelectObject(bmp)
            cDC.BitBlt((0, 0), (w, h), dcObj, (wx, wy), win32con.SRCCOPY)
            bits = bmp.GetBitmapBits(True)
            img = np.frombuffer(bits, dtype=np.uint8).reshape(h, w, 4)
            result = img[:, :, :3].copy()
            dcObj.DeleteDC()
            cDC.DeleteDC()
            win32gui.ReleaseDC(self.hwnd, wDC)
            win32gui.DeleteObject(bmp.GetHandle())
            return result
        except:
            return None
    
    
    # --- Background Input Methods ---
    def get_client_coordinates(self, wx, wy):
        """Convert Window-Relative (from GetWindowDC) to Client-Relative (for PostMessage)"""
        screen_x = self.x + wx
        screen_y = self.y + wy
        try:
            client_point = win32gui.ScreenToClient(self.hwnd, (screen_x, screen_y))
            return client_point[0], client_point[1]
        except:
            return wx, wy # Fallback

    def background_click(self, wx, wy):
        cx, cy = self.get_client_coordinates(wx, wy)
        lparam = win32api.MAKELONG(cx, cy)
        win32gui.PostMessage(self.hwnd, win32con.WM_LBUTTONDOWN, win32con.MK_LBUTTON, lparam)
        time.sleep(0.05)
        win32gui.PostMessage(self.hwnd, win32con.WM_LBUTTONUP, 0, lparam)

    def background_move(self, wx, wy):
        cx, cy = self.get_client_coordinates(wx, wy)
        lparam = win32api.MAKELONG(cx, cy)
        win32gui.PostMessage(self.hwnd, win32con.WM_MOUSEMOVE, 0, lparam)

    def background_press(self, key):
        vk = 0
        if key == 'space': vk = win32con.VK_SPACE
        elif key == 'x': vk = ord('X')
        
        if vk:
            win32gui.PostMessage(self.hwnd, win32con.WM_KEYDOWN, vk, 0)
            time.sleep(0.05)
            win32gui.PostMessage(self.hwnd, win32con.WM_KEYUP, vk, 0)

    # --- Wrapper Methods ---
    def move(self, wx, wy):
        # [FIX] Game usually requires PHYSICAL mouse move to show nameplate (Red bar)
        # Virtual WM_MOUSEMOVE is not enough for UI triggers in most DirectX games.
        # We must use pyautogui for movement to ensure 'check_red_at' works.
        pyautogui.moveTo(self.x + wx, self.y + wy) 
        # self.background_move(wx, wy) # Disabled: Doesn't trigger nameplate
    
    def click(self, wx, wy):
        # Physical click is more reliable for target locking
        pyautogui.click(self.x + wx, self.y + wy)
        # self.background_click(wx, wy)

    def press(self, key):
        # Keyboard usually works fine in background
        self.background_press(key)
        
    def count_red(self, region):
        if region is None:
            return 0
        mask = cv2.inRange(region, np.array([0, 0, 130]), np.array([80, 80, 255]))
        return cv2.countNonZero(mask)
    
    def detect_monsters_yolo(self, frame):
        """Detect monsters using YOLO"""
        if not self.use_yolo or self.model is None:
            return []
        if frame is None:
            return []

        # SAVE DEBUG FRAME (Once)
        if not hasattr(self, 'debug_saved'):
            cv2.imwrite("debug_view_bot.jpg", frame)
            print("[DEBUG] Saved debug_view_bot.jpg - Check this image!")
            self.debug_saved = True

        # Run inference
        results = self.model(frame, verbose=False, conf=0.1)  # Force low conf
        
        detections = []
        cx = self.width // 2
        cy = self.height // 2
        
        for r in results:
            boxes = r.boxes
            # Debug: show total raw detections
            if len(boxes) > 0:
                print(f"  [DEBUG] Raw boxes: {len(boxes)}")
            for box in boxes:
                conf = box.conf.item()
                if conf < self.confidence:
                    continue
                
                # Get box coordinates
                x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)
                
                # Center
                bx = (x1 + x2) // 2
                by = (y1 + y2) // 2
                
                # Distance from player
                dx = bx - cx
                dy = by - cy
                dist = math.sqrt(dx*dx + dy*dy)
                
                # Skip player area
                if dist < 80:
                    continue
                
                detections.append({
                    'x1': x1, 'y1': y1,
                    'x2': x2, 'y2': y2,
                    'cx': bx, 'cy': by,
                    'conf': conf,
                    'dist': dist
                })
        
        # Sort by distance
        detections.sort(key=lambda d: d['dist'])
        return detections
    
    def check_red_at(self, wx, wy):
        self.move(wx, wy)
        # Scan area lebih fokus (Nameplate biasanya di ATAS monster)
        time.sleep(0.05)
        
        # Center (wx, wy)
        # Scan width: 80 (wx-40 to wx+40)
        # Scan height: 60 (wy-90 to wy-30) -> Asumsi nameplate di atas center body
        x = max(0, wx - 40)
        y = max(0, wy - 90)
        region = self.capture_region(x, y, 80, 60)
        
        red_count = self.count_red(region)
        # print(f"  [DEBUG] Red pixels at ({wx},{wy}): {red_count}") # Uncomment for deeper debug
        
        # Threshold 15 pixel (Nameplate bar tipis pun kena)
        return red_count > 15

    def attack_target(self, wx, wy):
        global STOP_FLAG
        
        # Initial Attack
        self.click(wx, wy)
        time.sleep(0.1)
        self.press('space')
        
        start = time.time()
        hits = 0
        missed_scans = 0
        max_miss = 15  # Lebih sabar (tadi cuma 2)
        
        while not STOP_FLAG:
            now = time.time()
            
            # Timeout combat 30s
            if now - start > 30:
                print("  [TIMEOUT] Combat too long")
                return False
            
            # Spam attack
            self.press('space')
            hits += 1
            time.sleep(0.15)  # Lebih cepat spamnya
            
            # Cek status monster setiap 3 hit (lebih responsif)
            if hits % 3 == 0:
                has_red = False
                
                # Scan di titik awal
                if self.check_red_at(wx, wy):
                    has_red = True
                    missed_scans = 0 # Reset jika ketemu
                
                if not has_red:
                    missed_scans += 1
                    
                    if missed_scans >= 3: # Cukup 3x miss (sekitar 1.5 detik)
                        print(f"  [KILLED] Target lost/dead after {hits} hits")
                        return True
                else:
                    missed_scans = 0
        
        return False
    
    def do_loot(self, wx, wy):
        global STOP_FLAG
        print("  [LOOTING] Fast loot...")
        
        # Center
        self.move(wx, wy)
        self.press('x') 
        self.press('space')
        
        # Spiral offsets (Reduced count and sleep for speed)
        offsets = [
            (-30, 0), (30, 0), (0, -30), (0, 30),
            (-30, -30), (30, -30), (-30, 30), (30, 30)
        ]
        
        for dx, dy in offsets:
            if STOP_FLAG: return
            self.move(wx + dx, wy + dy)
            self.press('x')
            self.press('space')
            time.sleep(0.01) # Ultra fast
        
        print("  [LOOTING] Done.") 
        time.sleep(0.1)
    
    def draw_detection(self, x1, y1, x2, y2, conf):
        """Draw detection box on screen (optional overlay)"""
        try:
            hdc = win32gui.GetWindowDC(self.hwnd)
            pen = win32gui.CreatePen(win32con.PS_SOLID, 2, win32api.RGB(0, 255, 0))
            old = win32gui.SelectObject(hdc, pen)
            brush = win32gui.GetStockObject(win32con.NULL_BRUSH)
            win32gui.SelectObject(hdc, brush)
            win32gui.Rectangle(hdc, x1, y1, x2, y2)
            win32gui.SelectObject(hdc, old)
            win32gui.DeleteObject(pen)
            win32gui.ReleaseDC(self.hwnd, hdc)
        except:
            pass
    
    def run(self):
        global STOP_FLAG
        
        print("=" * 50)
        print("  RF AUTO BOT v16.0")
        print("  YOLO OBJECT DETECTION")
        print("=" * 50)
        
        if not self.find_window():
            print("[ERROR] RF Online not found!")
            input("Press Enter...")
            return
        
        print(f"[OK] RF Online ({self.width}x{self.height})")
        
        # Try to load YOLO model
        if not self.load_yolo_model():
            print()
            print("[FALLBACK] Using hover scan mode")
            print("[TIP] To use YOLO:")
            print("  1. Run: python data_collector.py")
            print("  2. Collect 20+ training images")
            print("  3. Run: python train_model.py")
        
        print()
        print("[STOP] ESC atau Ctrl+C")
        print()
        print("Starting in 3 seconds...")
        time.sleep(3)
        
        kb_thread = threading.Thread(target=keyboard_listener, daemon=True)
        kb_thread.start()
        
        print("\n[RUNNING]")
        print("-" * 50)
        
        frame_count = 0
        
        try:
            while not STOP_FLAG:
                frame = self.capture_screen()
                if frame is None:
                    time.sleep(0.1)
                    continue
                
                frame_count += 1
                
                if self.use_yolo:
                    # YOLO Detection
                    detections = self.detect_monsters_yolo(frame)
                    
                    if frame_count % 30 == 0:
                        print(f"[YOLO] Detected: {len(detections)} | Kills: {self.kills}")
                    
                    for det in detections:
                        if STOP_FLAG:
                            break
                        
                        # Draw box
                        self.draw_detection(det['x1'], det['y1'], det['x2'], det['y2'], det['conf'])
                        
                # Verify with red nameplate
                x_check, y_check = det['cx'], det['cy']
                if self.check_red_at(x_check, y_check):
                    print(f"\n[MONSTER] ({x_check}, {y_check}) conf={det['conf']:.2f} dist={det['dist']:.1f}")
                    
                    if self.attack_target(x_check, y_check):
                        self.kills += 1
                        print(f"[KILLS] {self.kills}")
                        self.do_loot(x_check, y_check)
                        time.sleep(0.1)
                    break
                
                else:
                    pass
            
            else:
                # Fallback: Hover scan mode (Only if YOLO didn't run or disabled)
                if frame_count % 100 == 0:
                    print(f"[SCAN] Frame {frame_count} | Kills: {self.kills}")
                    
                    # Simple grid scan
                    for y in range(200, self.height - 200, 80):
                        for x in range(150, self.width - 150, 80):
                            if STOP_FLAG:
                                break
                            
                            # Skip center
                            dx = x - self.width // 2
                            dy = y - self.height // 2
                            if dx*dx + dy*dy < 8000:
                                continue
                            
                            if self.check_red_at(x, y):
                                print(f"\n[MONSTER] ({x}, {y})")
                                
                                if self.attack_target(x, y):
                                    self.kills += 1
                                    print(f"[KILLS] {self.kills}")
                                    self.do_loot(x, y)
                                    time.sleep(0.2)
                                break
                        else:
                            continue
                        break
                
                time.sleep(0.02)
        
        except KeyboardInterrupt:
            print("\n[Ctrl+C]")
            STOP_FLAG = True
        
        print(f"\n[DONE] Kills: {self.kills}")


if __name__ == "__main__":
    bot = RFBot()
    bot.run()
