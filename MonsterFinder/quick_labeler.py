"""
Simple Quick Labeler v2
Label gambar yang sudah ada di dataset/images
"""

import cv2
import os
import glob

class Labeler:

    def __init__(self):
        self.base = os.path.dirname(os.path.abspath(__file__))
        self.img_dir = os.path.join(self.base, "dataset", "images")
        self.lbl_dir = os.path.join(self.base, "dataset", "labels")
        
        os.makedirs(self.img_dir, exist_ok=True)
        os.makedirs(self.lbl_dir, exist_ok=True)
        
        # [NEW] Class Definitions
        self.classes = {
            0: "Monster",
            1: "Character",
            2: "Loot",
            3: "UI_Chat",
            4: "UI_Map",
            5: "UI_Skill",
            6: "Ore/Mineral"
        }
        self.current_class = 0
        
        self.boxes = [] # [(x1, y1, x2, y2, class_id)]
        self.drawing = False
        self.sx, self.sy = 0, 0
        self.ex, self.ey = 0, 0
        
        self.model = None
        self.load_model()
        
    def load_model(self):
        try:
            from ultralytics import YOLO
            # Try to load best model
            model_path = os.path.join(self.base, "models", "monster_detector", "weights", "best.pt")
            if not os.path.exists(model_path):
                 model_path = os.path.join(self.base, "models", "monster_detector.pt")
            
            if os.path.exists(model_path):
                self.model = YOLO(model_path)
                print(f"[INFO] Loaded Auto-Label Model: {model_path}")
            else:
                print("[INFO] No trained model found (Auto-Label disabled)")
        except:
            print("[INFO] Ultralytics not found (Auto-Label disabled)")

    def auto_label(self, img):
        if self.model is None:
            print("[ERROR] No model loaded for Auto-Label")
            return
        
        print("  [AI] Detecting monsters...")
        results = self.model(img, verbose=False)
        
        count = 0
        for r in results:
            for box in r.boxes:
                # Get box coordinates
                x1, y1, x2, y2 = box.xyxy[0].cpu().numpy().astype(int)
                conf = box.conf.item()
                cls = int(box.cls.item())
                
                # Only if confidence > 0.4
                if conf > 0.4:
                    # Map OLD model class (0=monster) to NEW class system
                    # Assuming old model only knows 0=Monster
                    if cls == 0:
                        self.boxes.append((x1, y1, x2, y2, 0)) # 0 = Monster
                        count += 1
        
        print(f"  [AI] Added {count} boxes")
    
    def mouse(self, event, x, y, flags, param):
        if event == cv2.EVENT_LBUTTONDOWN:
            self.drawing = True
            self.sx, self.sy = x, y
        elif event == cv2.EVENT_MOUSEMOVE and self.drawing:
            self.ex, self.ey = x, y
        elif event == cv2.EVENT_LBUTTONUP:
            self.drawing = False
            x1, y1 = min(self.sx, x), min(self.sy, y)
            x2, y2 = max(self.sx, x), max(self.sy, y)
            if (x2-x1) > 5 and (y2-y1) > 5:
                # Store box with current class ID
                self.boxes.append((x1, y1, x2, y2, self.current_class))
                print(f"  Box {len(self.boxes)}: {self.classes[self.current_class]}")
    
    def save_label(self, img_path, h, w):
        basename = os.path.splitext(os.path.basename(img_path))[0]
        txt_path = os.path.join(self.lbl_dir, basename + ".txt")
        
        with open(txt_path, 'w') as f:
            for (x1, y1, x2, y2, cls_id) in self.boxes:
                cx = ((x1 + x2) / 2) / w
                cy = ((y1 + y2) / 2) / h
                bw = (x2 - x1) / w
                bh = (y2 - y1) / h
                f.write(f"{cls_id} {cx:.6f} {cy:.6f} {bw:.6f} {bh:.6f}\n")
        
        print(f"[SAVED] {txt_path} ({len(self.boxes)} boxes)")
        return True
    
    def run(self):
        print("=" * 50)
        print("  MULTI-CLASS LABELER v3")
        print("=" * 50)
        
        # Get images
        images = glob.glob(os.path.join(self.img_dir, "*.png"))
        images += glob.glob(os.path.join(self.img_dir, "*.jpg"))
        
        if not images:
            print("[ERROR] No images in dataset/images/")
            return
        
        print(f"[FOUND] {len(images)} images")
        print()
        print("[CONTROLS]")
        print("  Click+Drag = Draw box")
        print("  1-7 = Switch Class (Monster, Char, Loot, etc)")
        print("  S = Save and next")
        print("  R = Remove last box")
        print("  C = Clear all boxes")
        print("  N = Skip")
        print("  Q = Quit")
        print()
        
        cv2.namedWindow("Labeler", cv2.WINDOW_NORMAL)
        cv2.setMouseCallback("Labeler", self.mouse)
        
        labeled = 0
        
        colors = [
            (0, 0, 255),    # 0: Monster (Red)
            (255, 0, 0),    # 1: Character (Blue)
            (0, 255, 255),  # 2: Loot (Yellow)
            (255, 255, 0),  # 3: Chat (Cyan)
            (255, 0, 255),  # 4: Map (Magenta)
            (0, 255, 0),    # 5: Skill (Green)
            (128, 128, 128) # 6: Ore (Gray)
        ]
        
        for img_path in images:
            self.boxes = []
            img = cv2.imread(img_path)
            if img is None: continue
            
            h, w = img.shape[:2]
            filename = os.path.basename(img_path)
            
            # Check existing labels
            basename = os.path.splitext(filename)[0]
            label_path = os.path.join(self.lbl_dir, basename + ".txt")
            if os.path.exists(label_path):
                # Optional: Load existing labels to edit? For now just skip or clear
                # print(f"[SKIP] {filename} already labeled")
                # continue
                pass 

            print(f"\n[IMAGE] {filename}")
            
            while True:
                disp = img.copy()
                
                # Draw boxes
                for i, (x1, y1, x2, y2, cls_id) in enumerate(self.boxes):
                    color = colors[cls_id] if cls_id < len(colors) else (255,255,255)
                    cv2.rectangle(disp, (x1, y1), (x2, y2), color, 2)
                    cv2.putText(disp, f"{i+1}:{self.classes[cls_id]}", (x1, y1-5), 
                               cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)
                
                # Draw drawing box
                if self.drawing:
                    cv2.rectangle(disp, (self.sx, self.sy), (self.ex, self.ey), (255, 255, 255), 1)
                
                # UI Info
                current_color = colors[self.current_class] if self.current_class < len(colors) else (255,255,255)
                status_text = f"ACTIVE: [{self.current_class}] {self.classes[self.current_class]}"
                cv2.putText(disp, status_text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.8, current_color, 2)
                
                help_text = "keys: 1=Mon, 2=Char... | S=Save | A=Auto-Detect"
                cv2.putText(disp, help_text, (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)
                
                cv2.imshow("Labeler", disp)
                key = cv2.waitKey(20) & 0xFF
                
                if key == ord('q'):
                    cv2.destroyAllWindows()
                    return
                elif key == ord('n'): break
                elif key == ord('c'): self.boxes = []
                elif key == ord('r') and self.boxes: self.boxes.pop()
                elif key == ord('a'): self.auto_label(img) # Trigger Auto-Label
                elif key == ord('s'):
                    if self.boxes:
                        self.save_label(img_path, h, w)
                        labeled += 1
                        break
                    else:
                        print("  [No boxes!]")
                
                # Class Switching
                if ord('1') <= key <= ord('7'):
                    self.current_class = key - ord('1')
                    print(f"  [Switched] {self.classes[self.current_class]}")

        cv2.destroyAllWindows()
        print(f"\n[DONE] Labeled {labeled} images")
        print(f"[TIP] Now run: python train_model.py")


if __name__ == "__main__":
    labeler = Labeler()
    labeler.run()
