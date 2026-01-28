#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <map>
#include <queue>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <functional>
#include <thread>
#include <mutex>

using namespace std;

// ==========================================
//        ENHANCED CONFIGURATION
// ==========================================

struct ColorRGB {
    int r, g, b;
    int tolerance;
    string name;
    
    bool Matches(int r2, int g2, int b2) const {
        return (abs(r - r2) <= tolerance && 
                abs(g - g2) <= tolerance && 
                abs(b - b2) <= tolerance);
    }
};

struct MonsterInfo {
    string name;
    ColorRGB color;
    int priority;            // 1 = highest, 10 = lowest
    bool aggressive;
    int hpBarOffset;         // Distance from top of box to HP bar
    int estimatedHP;
    int expReward;
    bool ignore;
};

struct SkillInfo {
    string name;
    WORD key;
    int cooldownMs;
    int priority;
    int minHPPercentToUse;
    int maxHPPercentToUse;
    DWORD lastUsedTime;
    bool enabled;
};

struct PotionConfig {
    WORD hpKey;
    WORD mpKey;
    int hpPercentThreshold;
    int mpPercentThreshold;
    int useDelayMs;
    DWORD lastUseTime;
    bool enabled;
};

struct BotConfig {
    // Scanning
    int scanWidth;
    int scanHeight;
    int scanStep;
    int playerSafeRadius;
    int motionThreshold;
    int minBoxSize;
    int mergeDistance;
    
    // Attack
    int maxAttackTime;
    int attackCooldownMs;
    bool autoSkill;
    int attackDelayMs;
    bool chainAttack;
    
    // Targeting
    string targetMode; // "nearest", "strongest", "weakest", "priority"
    bool onlyAggressive;
    bool avoidElite;
    
    // Movement
    bool autoLoot;
    int autoLootKey;
    int autoLootIntervalMs;
    bool avoidPlayers;
    int playerAvoidDistance;
    
    // Safety
    int playerHPThreshold;
    WORD escapeKey;
    bool autoEscape;
    bool pauseOnLowHP;
    
    // Advanced
    bool randomDelay;
    int randomDelayMin;
    int randomDelayMax;
    bool humanLikeBehavior;
    int clickDeviation;
    int clickDelayMin;
    int clickDelayMax;
    
    // Display
    bool showBoxes;
    bool showOverlay;
    bool showDebugInfo;
    bool showESPLines;        // Show ESP lines from player to monsters
    bool enablePathfinding;   // Enable pathfinding following ESP lines
    int maxESPLines;          // Maximum ESP lines to show (prioritize nearest)
    
    // Detection (advanced)
    bool debugDetection;      // Enable debug logging for detection
    int scanQuality;          // Scan quality: 1=strict, 2=normal, 3=relaxed
    int detectionSensitivity; // Detection sensitivity: 1=low, 2=medium, 3=high
    bool adaptiveThresholds; // Auto-adjust thresholds based on conditions
    int confidenceLevel;       // Detection confidence: 1-10
    
    // Cursor-based detection
    bool useCursorDetection;  // Use cursor color detection (red = monster)
    int cursorRedThreshold;   // Red threshold for cursor detection (default: 180)
    int cursorGreenMax;       // Max green for red cursor (default: 100)
    int cursorBlueMax;        // Max blue for red cursor (default: 100)
    bool autoPressX;          // Auto-press X key every 2 seconds
    int autoPressXInterval;   // Interval in ms for auto-press X (default: 2000)
    WORD autoPressXKey;       // Key code for auto-press (default: 'X')
};

struct Statistics {
    int totalKills;
    int totalExp;
    int itemsLooted;
    int potionsUsed;
    DWORD startTime;
    DWORD runningTime;
    int monstersPerMinute;
    double expPerMinute;
};

// ==========================================
//        DATA STRUCTURES
// ==========================================

struct TargetPos {
    int x, y;
    double dist;
    ColorRGB color;
};

struct BoundingBox {
    int minX, maxX, minY, maxY;
    int pixelCount;
    int centerX, centerY;
    double distToCenter;
    ColorRGB color;
    int estimatedHP;
    string monsterName;
    int priority;
    bool isAggressive;
    int trackingID;  // Unique ID for tracking
    DWORD lastSeenTime;  // Last time this monster was detected
    bool isAlive;  // Whether monster is still alive
};

struct PlayerStatus {
    int currentHP;
    int maxHP;
    int currentMP;
    int maxMP;
    int level;
    int currentExp;
    int expToNext;
    
    double GetHPPercent() const {
        return maxHP > 0 ? (double)currentHP / maxHP * 100.0 : 100.0;
    }
    
    double GetMPPercent() const {
        return maxMP > 0 ? (double)currentMP / maxMP * 100.0 : 100.0;
    }
    
    bool IsLowHP() const {
        return GetHPPercent() < 30.0;
    }
    
    bool IsCriticalHP() const {
        return GetHPPercent() < 15.0;
    }
};

// ==========================================
//        HOTKEY SYSTEM
// ==========================================

enum HotkeyAction {
    HK_START_BOT,
    HK_STOP_BOT,
    HK_PAUSE_BOT,
    HK_TOGGLE_AUTO_LOOT,
    HK_TOGGLE_SHOW_BOXES,
    HK_USE_POTION,
    HK_ESCAPE,
    HK_TOGGLE_SKILL,
    HK_RELOAD_CONFIG,
    HK_QUICK_TARGET,
    HK_CLEAR_TARGET,
    HK_SAVE_CONFIG
};

struct Hotkey {
    WORD key;
    HotkeyAction action;
    bool enabled;
    string description;
};

// ==========================================
//        ENHANCED SCANNER
// ==========================================

class EnhancedScanner {
private:
    HWND hwnd;
    HDC hdcWindow;
    HDC hdcMem;
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    BYTE* pBits;
    BYTE* pPrevBits;
    int width, height;
    int bufferSize;
    DWORD lastCaptureTime;
    int frameRate;
    
public:
    EnhancedScanner(HWND targetHwnd, int w, int h);
    ~EnhancedScanner();
    
    void Capture();
    bool GetPixel(int x, int y, int& r, int& g, int& b) const;
    bool CheckPixelWithColor(const ColorRGB& color, int x, int y, bool checkMotion = true);
    bool CheckPixelWithMotion(int x, int y, int& r, int& g, int& b, int threshold);
    
    vector<TargetPos> FindColorTargets(const vector<ColorRGB>& colors, bool checkMotion = true);
    BoundingBox CreateBoundingBox(const vector<TargetPos>& pixels, const ColorRGB& color);
    
    int GetFrameRate() const { return frameRate; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
};

// ==========================================
//        MONSTER DATABASE
// ==========================================

class MonsterDatabase {
private:
    map<string, MonsterInfo> monsters;
    vector<ColorRGB> allColors;
    
public:
    void LoadDefaultMonsters();
    void AddMonster(const MonsterInfo& monster);
    void RemoveMonster(const string& name);
    MonsterInfo* GetMonster(const string& name);
    vector<MonsterInfo> GetAllMonsters() const;
    vector<ColorRGB> GetAllColors() const;
    void LoadFromFile(const string& filename);
    void SaveToFile(const string& filename);
};

// ==========================================
//        SKILL MANAGER
// ==========================================

class SkillManager {
private:
    vector<SkillInfo> skills;
    int currentSkillIndex;
    bool rotationEnabled;
    
public:
    SkillManager();
    void AddSkill(const SkillInfo& skill);
    void RemoveSkill(int index);
    SkillInfo* GetNextSkill(const PlayerStatus& player, const BoundingBox& target);
    void ResetCooldowns();
    void UpdateCooldowns();
    void EnableSkill(int index, bool enabled);
    vector<SkillInfo> GetAllSkills() const;
    void LoadDefaultSkills();
};

// ==========================================
//        CONFIGURATION MANAGER
// ==========================================

class ConfigManager {
private:
    BotConfig config;
    string configPath;
    
public:
    ConfigManager();
    void LoadConfig(const string& path = "");
    void SaveConfig(const string& path = "");
    BotConfig& GetConfig() { return config; }
    const BotConfig& GetConfig() const { return config; }
    void ResetToDefaults();
};

// ==========================================
//        STATISTICS MANAGER
// ==========================================

class StatsManager {
private:
    Statistics stats;
    
public:
    StatsManager();
    void StartSession();
    void EndSession();
    void RecordKill(const MonsterInfo& monster);
    void RecordLoot(int items);
    void RecordPotionUsed();
    void UpdateStats();
    Statistics GetStats() const;
    string GetFormattedStats() const;
    void SaveToFile(const string& filename);
};

// ==========================================
//        LOGGER
// ==========================================

enum LogLevel {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG
};

class Logger {
private:
    ofstream logFile;
    string logPath;
    bool consoleOutput;
    
public:
    Logger();
    ~Logger();
    void Init(const string& path, bool console = true);
    void Log(LogLevel level, const string& message);
    void Info(const string& message);
    void Warning(const string& message);
    void Error(const string& message);
    void Debug(const string& message);
private:
    string GetTimestamp();
    string LevelToString(LogLevel level);
};

// ==========================================
//        INPUT MANAGER
// ==========================================

class InputManager {
private:
    HWND targetWindow;
    int clickDeviation;
    int clickDelayMin;
    int clickDelayMax;
    bool randomDelay;
    
public:
    InputManager(HWND hwnd);
    
    void SetClickDeviation(int deviation);
    void SetClickDelay(int minMs, int maxMs);
    void SetRandomDelay(bool enabled);
    
    void Click(int x, int y);
    void KeyPress(WORD key);
    void KeyDown(WORD key);
    void KeyUp(WORD key);
    void SendString(const string& text);
    
private:
    int GetRandomClickDelay();
    int GetRandomDeviation();
};

// ==========================================
//        HOTKEY MANAGER
// ==========================================

class HotkeyManager {
private:
    map<WORD, Hotkey> hotkeys;
    bool enabled;
    
public:
    HotkeyManager();
    void RegisterHotkey(WORD key, HotkeyAction action, const string& description);
    void UnregisterHotkey(WORD key);
    void EnableHotkey(WORD key, bool enable);
    void SetAllEnabled(bool enabled);
    
    vector<HotkeyAction> CheckHotkeys();
    Hotkey* GetHotkey(WORD key);
    vector<Hotkey> GetAllHotkeys() const;
    
    void LoadDefaultHotkeys();
};

// ==========================================
//        MONSTER FINDER BOT (MAIN CLASS)
// ==========================================

class MonsterFinderBot {
private:
    HWND gameWindow;
    EnhancedScanner* scanner;
    MonsterDatabase monsterDB;
    SkillManager skillManager;
    ConfigManager configManager;
    StatsManager statsManager;
    Logger logger;
    InputManager* inputManager;
    HotkeyManager hotkeyManager;
    
    BotConfig config;
    PotionConfig potionConfig;
    PlayerStatus playerStatus;
    
    BoundingBox currentTarget;
    vector<BoundingBox> detectedBoxes;  // Store all detected boxes for stable display
    map<int, BoundingBox> trackedMonsters;  // Tracked monsters with ID as key
    int nextTrackingID;  // Next available tracking ID
    DWORD lastScanTime;  // Last scan time for tracking
    bool isRunning;
    bool isPaused;
    bool isAttacking;
    DWORD attackStartTime;
    DWORD lastLootTime;
    DWORD lastUpdateTime;
    DWORD lastNoMonsterLogTime;  // Prevent spam logging
    
    thread botThread;
    mutex mutexLock;
    
public:
    MonsterFinderBot();
    ~MonsterFinderBot();
    
    // Initialization
    bool Initialize(HWND hwnd);
    bool LoadAllConfigurations();
    void Shutdown();
    
    // Control
    void Start();
    void Stop();
    void Pause();
    void Resume();
    bool IsRunning() const { return isRunning; }
    bool IsPaused() const { return isPaused; }
    
    // Core Logic
    void BotLoop();
    void ScanAndTarget();
    void AttackTarget();
    bool GetCursorColor(int& r, int& g, int& b);  // Get color at cursor position
    bool IsCursorRed();  // Check if cursor is red (monster detected)
    void CheckPlayerHP();
    void UsePotions();
    void AutoLoot();
    
    // Target Selection
    BoundingBox SelectTarget(const vector<BoundingBox>& candidates);
    bool IsMonsterAlive(const BoundingBox& box);
    bool IsPlayerInSafeRange(const BoundingBox& box);
    
    // Skill System
    void ExecuteSkillRotation();
    
    // UI Integration
    void SetStatusCallback(function<void(const string&)> callback);
    void SetGUIElements(HWND statusLabel);
    
    // Statistics & Logging
    Statistics GetStats() const;
    string GetStatsString() const;
    void LogActivity(const string& message, LogLevel level = LOG_INFO);
    
    // Configuration Access
    BotConfig& GetConfig() { return config; }
    MonsterDatabase& GetMonsterDB() { return monsterDB; }
    SkillManager& GetSkillManager() { return skillManager; }
    
private:
    // Helper functions
    void UpdatePlayerStatus();
    void DrawBoundingBox(HDC hdc, const BoundingBox& box, COLORREF color, int offsetX, int offsetY);
    void DrawOverlay();
    DWORD ApplyRandomDelay(DWORD baseDelay);
    bool CheckHotkeys();
    void HandleHotkey(HotkeyAction action);
    
    // ESP Line & Pathfinding functions
    void DrawESPLines(HDC hdc, int offsetX, int offsetY);
    COLORREF GetESPLineColor(const BoundingBox& monster, int index);
    vector<BoundingBox> SortMonstersByDistance(const vector<BoundingBox>& monsters);
    void FollowESPPath(const BoundingBox& target);
    vector<POINT> CalculatePathPoints(int startX, int startY, int endX, int endY);
    
    // Tracking functions
    int MatchMonsterToTracked(const BoundingBox& newBox);
    void UpdateTrackedMonsters(const vector<BoundingBox>& detectedBoxes);
    void RemoveDeadMonsters();
};
