"""
RF Online YOLO Training - VPS VERSION (High Performance)
Gunakan script ini di VPS untuk training maksimal!
"""

import os
import yaml
import multiprocessing

def train_vps():
    """Train YOLO model with High Settings"""
    try:
        from ultralytics import YOLO
        import torch
    except ImportError:
        print("Please install requirements:")
        print("pip install ultralytics torch torchvision")
        return

    # Check GPU
    device = 'cuda' if torch.cuda.is_available() else 'cpu'
    print(f"[INFO] Training on: {device.upper()}")
    if device == 'cuda':
        print(f"[INFO] GPU: {torch.cuda.get_device_name(0)}")
        workers = 8
        batch = 16
    else:
        print("[WARNING] No GPU found! Training might be slow.")
        workers = 4
        batch = 8

    # Dataset config
    base_dir = os.path.dirname(os.path.abspath(__file__))
    dataset_dir = os.path.join(base_dir, "dataset")
    
    # Force create/overwrite dataset.yaml with correct absolute paths
    yaml_path = os.path.join(dataset_dir, "dataset.yaml")
    
    # Absolute path to images directory
    images_dir = os.path.join(dataset_dir, "images")
    
    config = {
        'path': dataset_dir,    # Base path
        'train': 'images',      # Relative to path
        'val': 'images',        # Relative to path
        'names': {0: 'monster'}
    }
    
    print(f"[INFO] Creating dataset.yaml with path: {dataset_dir}")
    with open(yaml_path, 'w') as f:
        yaml.dump(config, f)
            
    # Load model (Start with Nano or Small)
    # yolov8n.pt = Nano (Cepat)
    # yolov8s.pt = Small (Lebih akurat)
    # yolov8m.pt = Medium (Sangat akurat, Berat)
    print("\n[LOADING] YOLOv8 Small model (Better Accuracy)...")
    model = YOLO('yolov8s.pt') 
    
    # Training
    print("\n[TRAINING] STARTING FULL POWER TRAINING...")
    print("=" * 50)
    
    results = model.train(
        data=yaml_path,
        epochs=150,             # Training lama (150 epoch)
        imgsz=640,              # Full HD detail
        batch=batch,            # Batch size sesuai hardware
        patience=20,            # Early stop patience
        device=device,          # Auto detect GPU
        workers=workers,        # Multi-thread data loading
        project='models',
        name='monster_detector',
        exist_ok=True,
        verbose=True,
        cache=True,             # Cache to RAM
        cos_lr=True,            # Cosine Learning Rate schedule
        optimizer='auto',       # Auto optimizer (AdamW usually)
    )
    
    print("\n" + "=" * 50)
    print("Training Complete!")
    print(f"Best model saved to: {base_dir}/models/monster_detector/weights/best.pt")
    print("=" * 50)

if __name__ == "__main__":
    train_vps()
