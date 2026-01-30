"""
RF Online YOLO Training Script - LITE Version
Train model untuk deteksi monster (optimized for CPU)
"""

import os
import yaml

def create_dataset_yaml():
    """Create dataset.yaml for YOLO training"""
    base_dir = os.path.dirname(os.path.abspath(__file__))
    dataset_dir = os.path.join(base_dir, "dataset")
    
    config = {
        'path': dataset_dir,
        'train': 'images',
        'val': 'images',
        'names': {
            0: 'monster'
        }
    }
    
    yaml_path = os.path.join(dataset_dir, "dataset.yaml")
    with open(yaml_path, 'w') as f:
        yaml.dump(config, f, default_flow_style=False)
    
    print(f"[OK] Created {yaml_path}")
    return yaml_path


def check_labels():
    """Check if labels are valid"""
    base_dir = os.path.dirname(os.path.abspath(__file__))
    labels_dir = os.path.join(base_dir, "dataset", "labels")
    images_dir = os.path.join(base_dir, "dataset", "images")
    
    if not os.path.exists(labels_dir):
        print("[ERROR] No labels folder!")
        return False
    
    label_files = [f for f in os.listdir(labels_dir) if f.endswith('.txt')]
    image_files = [f for f in os.listdir(images_dir) if f.endswith(('.jpg', '.png'))]
    
    print(f"[INFO] Images: {len(image_files)}")
    print(f"[INFO] Labels: {len(label_files)}")
    
    # Check if labels have content
    valid_labels = 0
    empty_labels = 0
    
    for lf in label_files:
        path = os.path.join(labels_dir, lf)
        with open(path, 'r') as f:
            content = f.read().strip()
            if content:
                valid_labels += 1
            else:
                empty_labels += 1
    
    print(f"[INFO] Valid labels: {valid_labels}")
    print(f"[INFO] Empty labels: {empty_labels}")
    
    if valid_labels == 0:
        print()
        print("[ERROR] No valid labels found!")
        print("[TIP] Use data_collector.py and:")
        print("      1. Press F5 to capture")
        print("      2. DRAW BOUNDING BOXES on monsters (click + drag)")
        print("      3. Press S to save")
        return False
    
    return True


def train_model():
    """Train YOLO model - LITE version for CPU"""
    try:
        from ultralytics import YOLO
    except ImportError:
        print("[ERROR] ultralytics not installed!")
        print("Run: pip install ultralytics")
        return
    
    base_dir = os.path.dirname(os.path.abspath(__file__))
    dataset_dir = os.path.join(base_dir, "dataset")
    images_dir = os.path.join(dataset_dir, "images")
    models_dir = os.path.join(base_dir, "models")
    
    os.makedirs(models_dir, exist_ok=True)
    
    # Check labels
    if not check_labels():
        return
    
    # Create dataset yaml
    yaml_path = create_dataset_yaml()
    
    # Load pre-trained model
    print("\n[LOADING] YOLOv8 nano model...")
    model = YOLO('yolov8n.pt')
    
    # LITE Training settings for CPU
    print("\n[TRAINING] LITE mode for CPU...")
    print("-" * 50)
    
    results = model.train(
        data=yaml_path,
        epochs=50,              # Lebih lama biar pintar
        imgsz=480,              # Lebih detail
        batch=4,
        patience=0,             # DISABLE early stopping
        project=models_dir,
        name='monster_detector',
        exist_ok=True,
        verbose=True,
        workers=0,
        cache=True,
        amp=False,
        lr0=0.01,
        optimizer='SGD'
    )
    
    # Save model
    best_model = os.path.join(models_dir, "monster_detector", "weights", "best.pt")
    final_model = os.path.join(models_dir, "monster_detector.pt")
    
    if os.path.exists(best_model):
        import shutil
        shutil.copy(best_model, final_model)
        print(f"\n[DONE] Model saved: {final_model}")
    
    print("\n" + "=" * 50)
    print("Training complete!")
    print("=" * 50)


def test_model():
    """Test trained model"""
    try:
        from ultralytics import YOLO
    except ImportError:
        print("[ERROR] ultralytics not installed!")
        return
    
    base_dir = os.path.dirname(os.path.abspath(__file__))
    model_path = os.path.join(base_dir, "models", "monster_detector.pt")
    
    if not os.path.exists(model_path):
        print(f"[ERROR] Model not found: {model_path}")
        return
    
    print(f"[LOADING] {model_path}")
    model = YOLO(model_path)
    
    images_dir = os.path.join(base_dir, "dataset", "images")
    images = [f for f in os.listdir(images_dir) if f.endswith(('.jpg', '.png'))]
    
    if images:
        test_img = os.path.join(images_dir, images[0])
        print(f"[TESTING] {test_img}")
        
        results = model(test_img)
        
        for r in results:
            print(f"[RESULT] Found {len(r.boxes)} objects")
            for box in r.boxes:
                print(f"  - monster conf={box.conf.item():.2f}")


if __name__ == "__main__":
    print("=" * 50)
    print("  YOLO TRAINING - LITE VERSION")
    print("=" * 50)
    print()
    print("1 = Check labels")
    print("2 = Train model (LITE)")
    print("3 = Test model")
    print()
    
    choice = input("Enter choice: ").strip()
    
    if choice == '1':
        check_labels()
    elif choice == '2':
        train_model()
    elif choice == '3':
        test_model()
