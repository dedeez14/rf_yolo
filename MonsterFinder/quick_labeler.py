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
        
        self.boxes = []
        self.drawing = False
        self.sx, self.sy = 0, 0
        self.ex, self.ey = 0, 0
    
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
            if (x2-x1) > 10 and (y2-y1) > 10:
                self.boxes.append((x1, y1, x2, y2))
                print(f"  Box {len(self.boxes)}: ({x1},{y1}) to ({x2},{y2})")
    
    def save_label(self, img_path, h, w):
        # Get base name without extension
        basename = os.path.splitext(os.path.basename(img_path))[0]
        txt_path = os.path.join(self.lbl_dir, basename + ".txt")
        
        with open(txt_path, 'w') as f:
            for (x1, y1, x2, y2) in self.boxes:
                cx = ((x1 + x2) / 2) / w
                cy = ((y1 + y2) / 2) / h
                bw = (x2 - x1) / w
                bh = (y2 - y1) / h
                f.write(f"0 {cx:.6f} {cy:.6f} {bw:.6f} {bh:.6f}\n")
        
        print(f"[SAVED] {txt_path} ({len(self.boxes)} boxes)")
        return True
    
    def run(self):
        print("=" * 50)
        print("  SIMPLE LABELER v2")
        print("=" * 50)
        
        # Cleanup wrong files in labels (PNG shouldn't be there)
        for f in os.listdir(self.lbl_dir):
            if f.endswith('.png') or f.endswith('.jpg'):
                os.remove(os.path.join(self.lbl_dir, f))
                print(f"[CLEANUP] Removed {f} from labels/")
        
        # Get images
        images = glob.glob(os.path.join(self.img_dir, "*.png"))
        images += glob.glob(os.path.join(self.img_dir, "*.jpg"))
        
        if not images:
            print("[ERROR] No images in dataset/images/")
            return
        
        print(f"[FOUND] {len(images)} images")
        print()
        print("[CONTROLS]")
        print("  Click+Drag = Draw box on monster")
        print("  S = Save and next")
        print("  R = Remove last box")
        print("  C = Clear all boxes")
        print("  N = Skip (no save)")
        print("  Q = Quit")
        print()
        
        cv2.namedWindow("Labeler", cv2.WINDOW_NORMAL)
        cv2.setMouseCallback("Labeler", self.mouse)
        
        labeled = 0
        
        for img_path in images:
            self.boxes = []
            img = cv2.imread(img_path)
            if img is None:
                continue
            
            h, w = img.shape[:2]
            filename = os.path.basename(img_path)
            
            # Check if already has label
            basename = os.path.splitext(filename)[0]
            label_path = os.path.join(self.lbl_dir, basename + ".txt")
            if os.path.exists(label_path):
                print(f"[SKIP] {filename} already labeled")
                continue
            
            print(f"\n[IMAGE] {filename}")
            
            while True:
                disp = img.copy()
                
                # Draw saved boxes
                for i, (x1, y1, x2, y2) in enumerate(self.boxes):
                    cv2.rectangle(disp, (x1, y1), (x2, y2), (0, 255, 0), 2)
                    cv2.putText(disp, str(i+1), (x1, y1-5), 
                               cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
                
                # Draw current drawing
                if self.drawing:
                    cv2.rectangle(disp, (self.sx, self.sy), (self.ex, self.ey), 
                                 (0, 255, 255), 2)
                
                # Info
                cv2.putText(disp, f"{filename} | Boxes: {len(self.boxes)}", 
                           (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
                cv2.putText(disp, "S=Save | R=Undo | N=Skip | Q=Quit", 
                           (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (200, 200, 200), 1)
                
                cv2.imshow("Labeler", disp)
                key = cv2.waitKey(30) & 0xFF
                
                if key == ord('q'):
                    cv2.destroyAllWindows()
                    print(f"\n[DONE] Labeled {labeled} images")
                    return
                
                if key == ord('n'):
                    break
                
                if key == ord('c'):
                    self.boxes = []
                    print("  [Cleared]")
                
                if key == ord('r') and self.boxes:
                    self.boxes.pop()
                    print("  [Undo]")
                
                if key == ord('s'):
                    if self.boxes:
                        self.save_label(img_path, h, w)
                        labeled += 1
                        break
                    else:
                        print("  [No boxes!]")
        
        cv2.destroyAllWindows()
        print(f"\n[DONE] Labeled {labeled} images")
        print(f"[TIP] Now run: python train_model.py")


if __name__ == "__main__":
    labeler = Labeler()
    labeler.run()
