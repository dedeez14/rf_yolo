# MonsterFinder Enhanced v2.0 - Deployment Instructions

## ✅ Build Status: SUCCESS

Berhasil compile `MonsterFinderEnhanced.exe` dengan Visual Studio 2022!

## 📦 File yang Dibuat:

### Core Executable:
- ✅ `MonsterFinderEnhanced.exe` - Main bot executable (x64, 1.5MB+)

### Configuration Files:
- ✅ `monsterfinder_config.ini` - Bot configuration
- ✅ `monsters.ini` - Monster database (includes NaidHelper colors)

### Documentation:
- ✅ `README_ENHANCED.md` - Complete documentation
- ✅ `BUILD_INSTRUCTIONS.md` - Build guide

### Source Files:
- `MonsterFinderEnhanced.h` - Header file
- `MonsterFinderEnhanced.cpp` - Implementation
- `MainEnhanced.cpp` - GUI & main
- `MonsterFinderEnhanced.vcxproj` - VS2022 project file

### Build Scripts:
- `build_simple.bat` - Working build script (USED)
- `build.bat` - Alternative build script

## 🚀 Cara Menjalankan:

### Step 1: Pastikan File Lengkap
Copy file-file berikut ke folder yang sama dengan exe:
```
MonsterFinderEnhanced.exe
monsterfinder_config.ini
monsters.ini
```

### Step 2: Buka RF Online
Login ke game dan masuk ke karakter yang ingin digunakan untuk auto-hunting.

### Step 3: Jalankan Bot
Double-click `MonsterFinderEnhanced.exe` atau jalankan dari command prompt:
```cmd
MonsterFinderEnhanced.exe
```

### Step 4: Konfigurasi Monster Colors
1. Di bot window, klik tombol **"1. Set Monster Color"**
2. Arahkan mouse ke monster di game yang ingin di-deteksi
3. Klik kiri untuk capture warna
4. Ulangi untuk semua warna monster yang ingin di-deteksi

### Step 5: Start Bot
- Tekan **F5** atau klik tombol **"START BOT"**
- Bot akan mulai hunting monster dengan warna yang terdeteksi

## 🎮 Kontrol Bot:

### Hotkeys:
- **F5** - Start Bot
- **F6** - Stop Bot
- **F7** - Pause/Resume
- **F8** - Toggle Auto Loot
- **F9** - Toggle Show Boxes
- **F10** - Emergency Escape
- **F11** - Reload Configuration

### GUI Controls:
- **Set Monster Color** - Capture warna monster dari game
- **Reload Config** - Reload config files tanpa restart
- **Clear Log** - Hapus activity log
- **Start/Stop/Pause** - Kontrol bot
- **Checkboxes** - Toggle options real-time

## 🔧 Konfigurasi NaidHelper Monsters:

Sudah ditambahkan 7 warna NaidHelper ke `monsters.ini`:

| Hex Color | RGB | Priority | Aggressive |
|-----------|-----|----------|-------------|
| #000001 | 0,0,1 | 3 | Yes |
| #33CDFC | 51,205,252 | 2 | Yes |
| #000000 | 0,0,0 | 4 | No |
| #010000 | 1,0,0 | 3 | Yes |
| #61D5FB | 97,213,251 | 2 | Yes |
| #93E7FC | 147,231,252 | 1 (Highest) | Yes |
| #247BCA | 36,123,202 | 2 | Yes |

### Cara Menambah Monster Baru:
1. Edit `monsters.ini`
2. Tambah entry baru dengan format:
   ```ini
   Nama_Monster,R,G,B,Tolerance,Priority,Aggressive,Offset,EXP,Ignore
   ```
3. Tekan **F11** untuk reload config tanpa restart

## 📊 Fitur-fitur:

### Monster Detection:
- ✅ Multi-color detection (7 warna NaidHelper)
- ✅ Motion-based filtering
- ✅ Priority-based targeting
- ✅ HP bar detection
- ✅ Monster database management

### Combat System:
- ✅ Automatic skill rotation
- ✅ Cooldown management
- ✅ Chain attack
- ✅ Smart target switching
- ✅ HP-based skill usage

### Safety Features:
- ✅ Auto HP/MP potions
- ✅ Critical HP escape
- ✅ Low HP pause
- ✅ Timeout protection
- ✅ Anti-stuck mechanisms

### Anti-Detection:
- ✅ Random delays (50-200ms)
- ✅ Click deviation (±5 pixels)
- ✅ Human-like behavior
- ✅ Variable attack speeds

### Statistics:
- ✅ Real-time kill tracking
- ✅ EXP accumulation
- ✅ Loot counting
- ✅ Potion usage stats
- ✅ KPM (Kills Per Minute)
- ✅ EPM (EXP Per Minute)
- ✅ Session time tracking

### UI:
- ✅ Modern GUI
- ✅ Real-time status
- ✅ Activity log
- ✅ Statistics display
- ✅ Live configuration changes

## 🛡️ Troubleshooting:

### Bot tidak mendeteksi monster:
1. Cek warna di `monsters.ini`
2. Capture warna baru dengan "Set Monster Color"
3. Adjust `tolerance` jika warna tidak cocok
4. Increase `motionThreshold` jika monster terlewat

### Bot terlalu lambat:
1. Increase `scanStep` dari 3 ke 4
2. Reduce `scanWidth`/`scanHeight`
3. Disable `showOverlay`
4. Close aplikasi lain yang membebani CPU

### Bot salah target:
1. Check `targetMode` di config
2. Verify monster priorities
3. Adjust `playerSafeRadius`
4. Enable `onlyAggressive` untuk target agresif saja

### Game tidak merespon:
1. Increase `clickDelayMin`/`Max`
2. Increase `attackCooldownMs`
3. Check window focus
4. Verify game is not minimized

## 📝 Build Details:

- **Compiler**: Microsoft Visual C++ Compiler Version 19.50.35723
- **Platform**: x64
- **C++ Standard**: C++17
- **Runtime**: Multi-threaded Static (/MT)
- **Optimization**: O2
- **Subsystem**: CONSOLE
- **Libraries**: gdi32.lib, user32.lib, comctl32.lib

## 📦 File Size:
- `MonsterFinderEnhanced.exe`: ~1.5MB
- Static linked (no DLL dependencies)
- Self-contained executable

## 🎯 Rekomendasi Penggunaan:

### Untuk Hunting NaidHelper:
1. Gunakan mode `targetMode = priority`
2. Enable `autoSkill = true`
3. Set `chainAttack = true`
4. Enable `randomDelay = true`
5. `clickDeviation = 5-10`
6. `attackDelayMs = 100-200`

### Untuk Safety:
1. `playerHPThreshold = 30`
2. `autoEscape = true`
3. `pauseOnLowHP = true`
4. `maxAttackTime = 10000` (10 detik)
5. Setup potion keys: `hpKey = VK_NUMPAD1`

### Untuk Performance:
1. `scanStep = 3` (balance akurasi vs speed)
2. `mergeDistance = 60`
3. Disable `showDebugInfo = false`
4. Keep `showBoxes = true` (untuk monitoring)

## ⚠️ Important Notes:

1. **Game Version**: Dites dengan RF Online versi terbaru
2. **Window Title**: Harus persis "RF Online"
3. **Administrator**: Jalankan sebagai admin jika ada masalah
4. **Antivirus**: Mungkin perlu add ke whitelist
5. **Background**: Jangan minimize game saat bot berjalan
6. **Session Log**: Auto-save ke file log dengan timestamp

## 🚀 Next Steps:

Sekarang Anda bisa:
1. ✅ Jalankan bot: `MonsterFinderEnhanced.exe`
2. ✅ Setup monster colors sesuai game Anda
3. ✅ Start hunting dengan F5
4. ✅ Monitor lewat GUI atau log files
5. ✅ Adjust settings real-time tanpa restart

Selamat hunting! 🎮
