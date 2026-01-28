# Farming TAI - RF Online Bot

**Enhanced Monster Finder Bot dengan Smart Tracking System**

Bot otomatis untuk RF Online yang dapat mendeteksi, melacak, dan membunuh monster secara otomatis dengan sistem tracking yang cerdas.

## 🎯 Fitur Utama

### ✨ Core Features
- **Smart Monster Detection** - Deteksi monster berdasarkan warna dengan sistem tracking yang stabil
- **Multi-Monster Tracking** - Setiap monster memiliki box terpisah yang mengikuti pergerakan
- **Auto Attack** - Serang monster secara otomatis dengan skill rotation
- **Auto Loot** - Loot otomatis setelah monster mati
- **Smart Targeting** - Pilih target berdasarkan priority, distance, atau weakest
- **Box Visualization** - Visualisasi box untuk setiap monster yang terdeteksi

### 🚀 Advanced Features
- **Real-time Tracking** - Box mengikuti pergerakan monster secara real-time
- **Dead Monster Removal** - Box menghilang otomatis saat monster mati
- **Performance Tracking** - Statistik kinerja bot (kills, exp, loot, dll)
- **Human-like Behavior** - Random delays dan variasi untuk menghindari deteksi
- **Hotkey Support** - Kontrol bot dengan hotkey (F5-F12)
- **Configurable** - Semua parameter dapat dikonfigurasi melalui file INI

## 📋 Requirements

- Windows 10/11
- Visual Studio 2019/2022/2025 dengan C++ compiler
- RF Online game client
- Administrator privileges (untuk input simulation)

## 🔧 Build Instructions

### Quick Build
```bash
cd MonsterFinder
build_simple.bat
```

### Manual Build
```bash
# Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

# Compile
cl /EHsc /std:c++17 /O2 /MT MainEnhanced.cpp MonsterFinderEnhanced.cpp /Fe:MonsterFinderEnhanced.exe /link gdi32.lib user32.lib comctl32.lib /SUBSYSTEM:CONSOLE
```

## ⚙️ Configuration

### File Konfigurasi

1. **monsterfinder_config.ini** - Konfigurasi utama bot
2. **monsters.ini** - Database monster dengan warna dan prioritas

### Contoh Konfigurasi

```ini
[Scanning]
scanWidth=1400
scanHeight=700
scanStep=3
playerSafeRadius=100
motionThreshold=20
minBoxSize=8
mergeDistance=100

[Attack]
maxAttackTime=10000
attackCooldownMs=500
autoSkill=true
chainAttack=true

[Targeting]
targetMode=priority
onlyAggressive=false

[Movement]
autoLoot=true
autoLootKey=88  ; X key
autoLootIntervalMs=2000
```

### Menambahkan Monster Baru

Edit `monsters.ini`:
```
MonsterName,R,G,B,Tolerance,Priority,Aggressive,HPOffset,Exp,Ignore
Bellato_Accretian,50,100,200,45,5,1,50,500,0
```

## 🎮 Hotkeys

| Hotkey | Action |
|--------|--------|
| **F5** | Start Bot |
| **F6** | Stop Bot |
| **F7** | Pause/Resume Bot |
| **F8** | Toggle Auto Loot |
| **F9** | Toggle Show Boxes |
| **F10** | Emergency Escape |
| **F11** | Reload Configuration |

## 📊 Statistics

Bot mencatat statistik berikut:
- Total Kills
- Total EXP Gained
- Items Looted
- Potions Used
- Monsters Per Minute
- EXP Per Minute

## 🛠️ Troubleshooting

### Bot tidak mendeteksi monster
1. Pastikan warna monster sudah benar di `monsters.ini`
2. Kurangi `minBoxSize` di config (default: 8)
3. Kurangi `playerSafeRadius` (default: 100)
4. Aktifkan `debugDetection=true` untuk melihat log

### Bot stuck setelah kill monster
- Pastikan `autoLoot=true` di config
- Periksa log untuk error messages
- Restart bot jika diperlukan

### Box tidak muncul atau kedip-kedip
- Pastikan `showBoxes=true` di config
- Periksa apakah game window dalam fokus
- Restart bot untuk refresh overlay

## 📁 Project Structure

```
MonsterFinder/
├── MonsterFinderEnhanced.h      # Header utama dengan semua class definitions
├── MonsterFinderEnhanced.cpp     # Implementasi core bot logic
├── MainEnhanced.cpp              # GUI dan main entry point
├── MonsterFinder.cpp             # Versi sederhana (legacy)
├── monsters.ini                  # Database monster
├── monsterfinder_config.ini      # Konfigurasi bot
├── build_simple.bat              # Script build sederhana
└── README.md                     # Dokumentasi ini
```

## 🔒 Safety Features

- **HP Threshold** - Bot akan pause jika HP terlalu rendah
- **Auto Escape** - Escape otomatis saat dalam bahaya
- **Timeout Protection** - Skip target jika attack terlalu lama
- **Player Safe Zone** - Tidak akan attack monster terlalu dekat dengan player

## 📝 Changelog

### Version 2.0 (Current)
- ✅ Smart tracking system dengan ID unik per monster
- ✅ Box mengikuti pergerakan monster secara real-time
- ✅ Auto loot setelah kill monster
- ✅ Improved detection dengan adaptive thresholds
- ✅ Enhanced UI dengan modern design
- ✅ Performance tracking dan statistics

### Version 1.0
- Basic monster detection
- Simple attack system
- Manual targeting

## 🤝 Contributing

Kontribusi sangat diterima! Silakan:
1. Fork repository
2. Buat feature branch
3. Commit perubahan
4. Push ke branch
5. Buat Pull Request

## 📄 License

This project is for educational purposes only. Use at your own risk.

## 👤 Author

**DedeProjectDev**
- GitHub: [@dedeez14](https://github.com/dedeez14)
- Project: Farming TAI - RF Online Bot

## 🙏 Acknowledgments

- RF Online community
- Open source contributors
- Beta testers

---

**⚠️ Disclaimer**: Bot ini dibuat untuk tujuan edukasi. Penggunaan bot dapat melanggar Terms of Service game. Gunakan dengan risiko sendiri.
