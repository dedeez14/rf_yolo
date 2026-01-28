# RF Hunter - Enhanced Version v2.0

Bot RF Online yang sangat powerful dengan fitur-fitur canggih untuk auto-hunting monster dengan efisiensi maksimal.

## 🚀 Fitur Utama

### 1. **Sistem Deteksi Monster Canggih**
- ✅ Deteksi berdasarkan warna + gerakan (motion detection)
- ✅ Mendukung multiple monster dengan warna berbeda
- ✅ Database monster yang dapat dikustomisasi
- ✅ Sistem prioritas target (nearest, strongest, weakest, priority)
- ✅ Filter monster agresif vs pasif
- ✅ Deteksi HP bar untuk memantau status monster

### 2. **Sistem Skill Otomatis**
- ✅ Rotasi skill otomatis dengan cooldown management
- ✅ Konfigurasi skill individual (cooldown, prioritas, kondisi HP)
- ✅ Kondisi penggunaan skill berdasarkan HP player
- ✅ Skill buff terpisah dengan cooldown panjang

### 3. **Manajemen HP/MP Otomatis**
- ✅ Auto-use potion saat HP/MP rendah
- ✅ Threshold konfigurasi
- ✅ Delay anti-spam
- ✅ Statistik penggunaan potion

### 4. **Keamanan & Anti-Deteksi**
- ✅ Random delay untuk perilaku lebih natural
- ✅ Click deviation (ketidakpastian posisi klik)
- ✅ Human-like behavior simulation
- ✅ Auto-escape saat HP kritis
- ✅ Pause otomatis saat HP rendah

### 5. **Targeting & Prioritas**
- ✅ 4 mode targeting: Nearest, Strongest, Weakest, Priority
- ✅ Prioritas monster berdasarkan database
- ✅ Filter monster agresif saja
- ✅ Avoid elite monsters option

### 6. **Sistem Hotkey Lengkap**
- ✅ F5 - Start Bot
- ✅ F6 - Stop Bot
- ✅ F7 - Pause/Resume
- ✅ F8 - Toggle Auto Loot
- ✅ F9 - Toggle Show Boxes
- ✅ F10 - Emergency Escape
- ✅ F11 - Reload Config
- ✅ F12 - Quick Target

### 7. **Statistik & Logging**
- ✅ Real-time statistics (kills, EXP, loot rate)
- ✅ Kills per minute & EXP per minute
- ✅ Activity log lengkap
- ✅ Save/load statistics
- ✅ Timestamp untuk semua aktivitas

### 8. **GUI Modern**
- ✅ Interface yang user-friendly
- ✅ Real-time status display
- ✅ Statistik panel
- ✅ Activity log window
- ✅ Toggle options on-the-fly

### 9. **Konfigurasi Lengkap**
- ✅ File-based configuration (INI format)
- ✅ Monster database terpisah
- ✅ Hot-reload configuration tanpa restart
- ✅ Default configuration included

### 10. **Auto Loot**
- ✅ Automatic loot collection
- ✅ Configurable interval
- ✅ Customizable key binding

## 📁 File Structure

```
MonsterFinder/
├── MonsterFinderEnhanced.h      # Header dengan semua class dan struktur
├── MonsterFinderEnhanced.cpp    # Implementasi lengkap
├── MainEnhanced.cpp             # GUI dan main function
├── monsterfinder_config.ini     # Konfigurasi utama
├── monsters.ini                  # Database monster
├── README_ENHANCED.md            # Dokumentasi ini
└── logs/                        # Folder untuk log files (auto-created)
```

## 🔧 Installation & Build

### Requirements:
- Windows OS
- Visual Studio 2019+ atau MinGW
- RF Online game window

### Build dengan Visual Studio:
1. Buat new Win32 Project
2. Copy semua file ke project folder
3. Set C++ Language Standard ke C++17 atau lebih baru
4. Add linker libraries: `gdi32.lib`, `user32.lib`, `comctl32.lib`
5. Build dan Run

### Build dengan MinGW:
```bash
g++ -std=c++17 -o MonsterFinderEnhanced.exe MainEnhanced.cpp MonsterFinderEnhanced.cpp -lgdi32 -luser32 -lcomctl32 -static
```

## ⚙️ Konfigurasi

### 1. monsterfinder_config.ini

File ini berisi semua setting utama bot. Edit sesuai kebutuhan:

```ini
[Scanning]
scanWidth=1400              # Lebar area scan (pixels)
scanHeight=700              # Tinggi area scan (pixels)
scanStep=3                  # Langkah scan (lebih kecil = lebih akurat tapi lebih lambat)
playerSafeRadius=130        # Radius aman sekitar player (tidak target monster di area ini)
motionThreshold=30          # Sensitivitas deteksi gerakan (lebih kecil = lebih sensitif)
minBoxSize=15               # Ukuran minimum monster box (pixels)
mergeDistance=60            # Jarak maksimum untuk merge pixel jadi satu box

[Attack]
maxAttackTime=10000         # Maksimum waktu attack per monster (milliseconds)
attackCooldownMs=500        # Delay antar attack (milliseconds)
autoSkill=true             # Enable/disable auto skill
attackDelayMs=100           # Delay setelah klik target (milliseconds)
chainAttack=true            # Enable/disable chain attack (continuous attacks)

[Targeting]
targetMode=priority         # Mode: nearest, strongest, weakest, priority
onlyAggressive=false        # Hanya target monster agresif
avoidElite=true             # Hindari elite/high-level monsters

[Movement]
autoLoot=true               # Enable auto loot
autoLootKey=88              # Key untuk loot (ASCII value, 88 = 'X')
autoLootIntervalMs=2000     # Interval antar loot (milliseconds)
avoidPlayers=true           # Hindari player lain
playerAvoidDistance=200     # Jarak hindari player (pixels)

[Safety]
playerHPThreshold=30        # Threshold HP untuk potion/escape (%)
escapeKey=27                # Key untuk escape (27 = ESC)
autoEscape=true             # Auto escape saat HP rendah
pauseOnLowHP=true          # Pause bot saat HP kritis

[Advanced]
randomDelay=true            # Add random delay untuk natural behavior
randomDelayMin=50           # Minimum random delay (ms)
randomDelayMax=200          # Maximum random delay (ms)
humanLikeBehavior=true      # Enable human-like behavior
clickDeviation=5            # Deviation posisi klik (pixels)
clickDelayMin=100           # Minimum click delay (ms)
clickDelayMax=300           # Maximum click delay (ms)

[Display]
showBoxes=true              # Show bounding boxes di screen
showOverlay=true            # Show overlay dengan debug info
showDebugInfo=false         # Show detailed debug info
```

### 2. monsters.ini

Database monster format:
```
name,r,g,b,tolerance,priority,aggressive,hpOffset,exp,ignore
```

Contoh:
```ini
Bellato_Accretian,50,100,200,45,5,1,50,500,0
```

Penjelasan field:
- `name`: Nama monster
- `r,g,b`: Warna RGB (0-255)
- `tolerance`: Toleransi pencocokan warna (0-255)
- `priority`: Prioritas target (1=highest, 10=lowest)
- `aggressive`: 1=agresif, 0=pasif
- `hpOffset`: Jarak dari atas box ke HP bar
- `exp`: Reward EXP
- `ignore`: 1=abaikan, 0=attack

#### Cara Menambah Monster Baru:

1. Jalankan bot
2. Klik tombol "1. Set Monster Color"
3. Pindahkan kursor ke monster yang ingin ditambahkan
4. Klik kiri untuk capture warna
5. Catat nilai RGB yang ditampilkan
6. Edit monsters.ini dan tambahkan entry baru:
   ```ini
   My_Custom_Monster,R,G,B,45,5,1,50,600,0
   ```
7. Reload config dengan F11 atau tombol "2. Reload Config"

## 🎮 Cara Penggunaan

### Langkah Awal:

1. **Buka RF Online** dan login ke karakter
2. **Jalankan MonsterFinderEnhanced.exe**
3. **Configure monster colors** dengan:
   - Klik "1. Set Monster Color"
   - Arahkan ke monster di game
   - Klik kiri untuk capture warna
   - Atau edit langsung monsters.ini

4. **Review configuration** di monsterfinder_config.ini

5. **Press F5** atau klik "START BOT" untuk memulai

### Selama Bot Berjalan:

- **F6**: Stop bot kapan saja
- **F7**: Pause/Resume bot
- **F8**: Toggle auto loot on-the-fly
- **F9**: Toggle display boxes
- **F10**: Emergency escape (cancel current action)
- **F11**: Reload configuration tanpa restart

### Tips & Tricks:

1. **Optimasi Performa**:
   - Sesuaikan `scanStep` (lebih besar = lebih cepat)
   - Kurangi `scanWidth`/`scanHeight` jika tidak butuh coverage penuh
   - Disable `showOverlay` untuk performa lebih baik

2. **Anti-Detection**:
   - Enable `randomDelay` dan `humanLikeBehavior`
   - Set `clickDeviation` ke 5-10
   - Gunakan `randomDelayMin` dan `max` yang wajar (50-300ms)

3. **Efisiensi Hunting**:
   - Gunakan mode `targetMode=priority` untuk EXP optimal
   - Set monster priority sesuai level dan EXP reward
   - Enable `chainAttack` untuk killing lebih cepat
   - Configure skill rotation dengan cooldown yang baik

4. **Safety**:
   - Set `playerHPThreshold` ke 30-40%
   - Enable `autoEscape` dan `pauseOnLowHP`
   - Setup potion keys di `potionConfig`

5. **Looting**:
   - Set `autoLootIntervalMs` sesuai kebutuhan
   - Pastikan `autoLootKey` sesuai dengan game setting

## 🔍 Mode Targeting

### 1. **Priority** (Default)
Target monster berdasarkan prioritas di database. Monster dengan priority 1 akan diserang duluan.

### 2. **Nearest**
Target monster yang paling dekat dengan player.

### 3. **Strongest**
Target monster dengan HP tertinggi (perlu HP detection yang akurat).

### 4. **Weakest**
Target monster dengan HP terendah.

## 📊 Statistik

Bot menampilkan statistik real-time:

- **Running Time**: Durasi sesi hunting
- **Total Kills**: Total monster yang dikalahkan
- **Total EXP**: Total EXP yang didapat
- **Items Looted**: Jumlah item yang diloot
- **Potions Used**: Jumlah potion yang dipakai
- **Kills/Min**: Rata-rata kill per menit
- **EXP/Min**: Rata-rata EXP per menit

Statistik disimpan otomatis saat bot berhenti dan dapat dilihat di `stats.ini`.

## 📝 Logging

Semua aktivitas bot dicatat ke file log dengan format:
```
monsterfinder_YYYYMMDD_HHMMSS.log
```

Level log:
- **INFO**: Aktivitas normal (start, stop, kill, dll)
- **WARNING**: Peringatan (HP rendah, timeout, dll)
- **ERROR**: Error fatal
- **DEBUG**: Informasi debug detail

## 🛡️ Fitur Keamanan

### 1. Human-Like Behavior
- Random delays di semua aksi
- Click deviation untuk natural clicks
- Variable attack speeds

### 2. Safety Checks
- HP monitoring
- Auto escape saat kritis
- Pause saat HP rendah
- Timeout protection

### 3. Anti-Stuck
- Attack timeout untuk mencegah stuck
- Auto-skip target yang tidak bisa dikalahkan
- Smart target switching

## 🐛 Troubleshooting

### Bot tidak mendeteksi monster:
1. Check monster colors di monsters.ini
2. Adjust `tolerance` jika warna tidak cocok
3. Check `motionThreshold` - terlalu tinggi mungkin skip monster
4. Verify window name sesuai dengan "RF Online"

### Bot terlalu lambat:
1. Increase `scanStep` dari 3 ke 4 atau 5
2. Reduce `scanWidth`/`scanHeight`
3. Disable `showOverlay`
4. Increase `mergeDistance` untuk mengurangi processing

### Bot salah target:
1. Check `targetMode` setting
2. Verify monster priorities
3. Adjust `playerSafeRadius`
4. Enable `onlyAggressive` jika mau target agresif saja

### Game tidak merespon input:
1. Check window focus - bot akan auto-focus
2. Increase `clickDelayMin`/`Max`
3. Reduce attack frequency

### HP tidak terbaca:
1. Adjust `hpOffset` di monster database
2. Check HP bar color (default: R>180, G<100, B<100)
3. Increase scan area untuk HP

## 📈 Advanced Configuration

### Custom Skill Rotation

Edit skill di `SkillManager::LoadDefaultSkills()`:

```cpp
skill.name = "My Skill";
skill.key = VK_F7;              // Key binding
skill.cooldownMs = 5000;        // Cooldown dalam ms
skill.priority = 3;             // Prioritas (lebih kecil = lebih sering)
skill.minHPPercentToUse = 0;   // Min HP % untuk gunakan skill
skill.maxHPPercentToUse = 100; // Max HP % untuk gunakan skill
```

### Custom Potion Keys

Edit di `MonsterFinderBot::Initialize()`:

```cpp
potionConfig.hpKey = VK_NUMPAD1;  // Key untuk HP potion
potionConfig.mpKey = VK_NUMPAD2;  // Key untuk MP potion
potionConfig.hpPercentThreshold = 30;  // Use HP potion di 30%
potionConfig.mpPercentThreshold = 20;  // Use MP potion di 20%
```

## 🔒 Disclaimer

**⚠️ WARNING**: Bot ini dibuat untuk tujuan pembelajaran dan edukasi programming.

Penggunaan bot dalam game melanggar Terms of Service dari kebanyakan game online dan dapat mengakibatkan:
- Banned account
- Suspended account
- Lost progress
- Legal action dari developer

**Gunakan dengan risiko Anda sendiri!** Penulis tidak bertanggung jawab atas konsekuensi penggunaan bot ini.

## 🤝 Kontribusi

Jika Anda ingin berkontribusi:
1. Fork repository
2. Buat feature branch
3. Commit changes
4. Push ke branch
5. Buat Pull Request

## 📄 License

Project ini dibuat untuk tujuan edukasi. Gunakan dengan bijak dan tanggung jawab.

## 📞 Support

Jika mengalami masalah atau memiliki pertanyaan:
1. Check section Troubleshooting
2. Review configuration files
3. Check log files di folder logs/

## 🎯 Roadmap (Future Enhancements)

- [ ] Pathfinding system
- [ ] GPS map integration
- [ ] Auto-party system
- [ ] Advanced HP/MP detection
- [ ] Inventory management
- [ ] Auto-vendor system
- [ ] Remote control via web interface
- [ ] Machine learning untuk pattern recognition
- [ ] Multi-bot coordination

## 📝 Changelog

### v2.0 (Current Version)
- ✅ Complete rewrite with modular architecture
- ✅ Monster database system
- ✅ Skill rotation manager
- ✅ Enhanced configuration system
- ✅ Hotkey system
- ✅ Statistics & logging
- ✅ Anti-detection features
- ✅ Modern GUI
- ✅ File-based configuration

### v1.0 (Original)
- Basic color + motion detection
- Simple auto-attack
- Basic GUI

---

**Happy Hunting! 🎮**

*Dibuat dengan ❤️ untuk pembelajaran AI dan automation programming*
