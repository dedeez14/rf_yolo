"""
Integration Script - Connect Python Detector dengan C++ Bot
Menggunakan shared memory atau file untuk komunikasi
"""

import cv2
import numpy as np
import json
import time
import os
from monster_detector import LightweightMonsterDetector, load_monster_colors_from_ini

class BotIntegration:
    """Integrasi detector Python dengan bot C++"""
    
    def __init__(self, config_file: str = "detector_config.json"):
        self.detector = LightweightMonsterDetector("RF Online")
        self.config_file = config_file
        self.output_file = "detections.json"
        self.running = False
        
    def load_config(self) -> dict:
        """Load konfigurasi dari file"""
        default_config = {
            "scan_width": 1400,
            "scan_height": 700,
            "scan_interval_ms": 100,
            "min_box_size": 8,
            "player_safe_radius": 80,
            "use_motion": True,
            "use_color": True
        }
        
        try:
            with open(self.config_file, 'r') as f:
                config = json.load(f)
                default_config.update(config)
        except:
            pass
        
        return default_config
    
    def save_detections(self, detections: List[dict]):
        """Save detections ke file JSON untuk dibaca oleh bot C++"""
        data = {
            "timestamp": time.time(),
            "detections": detections,
            "count": len(detections)
        }
        
        with open(self.output_file, 'w') as f:
            json.dump(data, f, indent=2)
    
    def run_detection_loop(self):
        """Main detection loop"""
        if not self.detector.find_window():
            print("ERROR: RF Online window not found!")
            return False
        
        config = self.load_config()
        color_ranges = load_monster_colors_from_ini("monsters.ini")
        
        print("Starting detection loop...")
        print(f"Scan interval: {config['scan_interval_ms']}ms")
        print(f"Using motion: {config['use_motion']}, Using color: {config['use_color']}")
        
        self.running = True
        last_scan = 0
        
        try:
            while self.running:
                current_time = time.time() * 1000  # Convert to ms
                
                if current_time - last_scan < config['scan_interval_ms']:
                    time.sleep(0.01)
                    continue
                
                # Capture screen
                frame = self.detector.capture_screen(
                    width=config['scan_width'],
                    height=config['scan_height']
                )
                
                if frame is None:
                    continue
                
                # Detect
                detections = self.detector.detect_combined(
                    frame,
                    color_ranges if config['use_color'] else None,
                    use_motion=config['use_motion']
                )
                
                # Filter berdasarkan min_box_size
                filtered = []
                for det in detections:
                    if det['w'] >= config['min_box_size'] and det['h'] >= config['min_box_size']:
                        # Check safe radius
                        center_x = config['scan_width'] // 2
                        center_y = config['scan_height'] // 2
                        box_center_x = det['x'] + det['w'] // 2
                        box_center_y = det['y'] + det['h'] // 2
                        dist = np.sqrt((box_center_x - center_x)**2 + (box_center_y - center_y)**2)
                        
                        if dist >= config['player_safe_radius']:
                            filtered.append(det)
                
                # Save detections
                self.save_detections(filtered)
                
                last_scan = current_time
                
                # Log setiap 1 detik
                if int(current_time) % 1000 < config['scan_interval_ms']:
                    print(f"Detections: {len(filtered)} | Time: {time.strftime('%H:%M:%S')}")
        
        except KeyboardInterrupt:
            print("\nStopping detector...")
        finally:
            self.running = False
        
        return True


def main():
    """Main entry point"""
    integration = BotIntegration()
    integration.run_detection_loop()


if __name__ == "__main__":
    main()
