#include "MonsterFinderEnhanced.h"
#include <set>
#include <algorithm>

// ==========================================
//        ENHANCED SCANNER IMPLEMENTATION
// ==========================================

EnhancedScanner::EnhancedScanner(HWND targetHwnd, int w, int h) {
    hwnd = targetHwnd;
    width = w;
    height = h;
    bufferSize = width * height * 4;
    lastCaptureTime = 0;
    frameRate = 0;

    hdcWindow = GetDC(hwnd);
    hdcMem = CreateCompatibleDC(hdcWindow);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
    hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
    pPrevBits = new BYTE[bufferSize];
    memset(pPrevBits, 0, bufferSize);
}

EnhancedScanner::~EnhancedScanner() {
    delete[] pPrevBits;
    SelectObject(hdcMem, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);
}

void EnhancedScanner::Capture() {
    DWORD currentTime = GetTickCount();
    if (lastCaptureTime > 0) {
        DWORD diff = currentTime - lastCaptureTime;
        if (diff > 0) {
            frameRate = 1000 / diff;
        }
    }
    
    memcpy(pPrevBits, pBits, bufferSize);

    RECT rect;
    GetClientRect(hwnd, &rect);
    int winW = rect.right - rect.left;
    int winH = rect.bottom - rect.top;
    int startX = (winW / 2) - (width / 2);
    int startY = (winH / 2) - (height / 2);
    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;

    BitBlt(hdcMem, 0, 0, width, height, hdcWindow, startX, startY, SRCCOPY);
    lastCaptureTime = currentTime;
}

bool EnhancedScanner::GetPixel(int x, int y, int& r, int& g, int& b) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return false;
    
    int offset = (y * width + x) * 4;
    b = pBits[offset];
    g = pBits[offset + 1];
    r = pBits[offset + 2];
    return true;
}

bool EnhancedScanner::CheckPixelWithColor(const ColorRGB& color, int x, int y, bool checkMotion) {
    if (x < 0 || x >= width || y < 0 || y >= height) return false;

    int offset = (y * width + x) * 4;
    int b = pBits[offset];
    int g = pBits[offset + 1];
    int r = pBits[offset + 2];

    if (color.Matches(r, g, b)) {
        if (checkMotion) {
            int prevR = pPrevBits[offset + 2];
            int prevG = pPrevBits[offset + 1];
            int prevB = pPrevBits[offset];
            int diff = abs(r - prevR) + abs(g - prevG) + abs(b - prevB);
            return (diff > 30); // Default motion threshold
        }
        return true;
    }
    return false;
}

bool EnhancedScanner::CheckPixelWithMotion(int x, int y, int& r, int& g, int& b, int threshold) {
    if (x < 0 || x >= width || y < 0 || y >= height) return false;

    int offset = (y * width + x) * 4;
    b = pBits[offset];
    g = pBits[offset + 1];
    r = pBits[offset + 2];

    int prevR = pPrevBits[offset + 2];
    int prevG = pPrevBits[offset + 1];
    int prevB = pPrevBits[offset];
    int diff = abs(r - prevR) + abs(g - prevG) + abs(b - prevB);

    return (diff > threshold);
}

vector<TargetPos> EnhancedScanner::FindColorTargets(const vector<ColorRGB>& colors, bool checkMotion) {
    vector<TargetPos> targets;
    int r, g, b;

    for (int y = 0; y < height; y += 3) {
        for (int x = 0; x < width; x += 3) {
            GetPixel(x, y, r, g, b);
            
            for (const auto& color : colors) {
                if (color.Matches(r, g, b)) {
                    if (checkMotion) {
                        int prevR = pPrevBits[(y * width + x) * 4 + 2];
                        int prevG = pPrevBits[(y * width + x) * 4 + 1];
                        int prevB = pPrevBits[(y * width + x) * 4];
                        int diff = abs(r - prevR) + abs(g - prevG) + abs(b - prevB);
                        
                        if (diff > 30) {
                            double dist = sqrt(pow(x - width / 2, 2) + pow(y - height / 2, 2));
                            targets.push_back({ x, y, dist, color });
                            break;
                        }
                    } else {
                        double dist = sqrt(pow(x - width / 2, 2) + pow(y - height / 2, 2));
                        targets.push_back({ x, y, dist, color });
                        break;
                    }
                }
            }
        }
    }

    return targets;
}

BoundingBox EnhancedScanner::CreateBoundingBox(const vector<TargetPos>& pixels, const ColorRGB& color) {
    if (pixels.empty()) {
        BoundingBox empty;
        memset(&empty, 0, sizeof(BoundingBox));
        empty.color = color;
        empty.isAlive = true;
        return empty;
    }

    int minX = pixels[0].x, maxX = pixels[0].x;
    int minY = pixels[0].y, maxY = pixels[0].y;

    for (const auto& p : pixels) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    int centerX = (minX + maxX) / 2;
    int centerY = (minY + maxY) / 2;
    double distToCenter = sqrt(pow(centerX - width / 2, 2) + pow(centerY - height / 2, 2));

    BoundingBox box;
    box.minX = minX;
    box.maxX = maxX;
    box.minY = minY;
    box.maxY = maxY;
    box.pixelCount = (int)pixels.size();
    box.centerX = centerX;
    box.centerY = centerY;
    box.distToCenter = distToCenter;
    box.color = color;
    box.estimatedHP = 0;
    box.monsterName = "";
    box.priority = 0;
    box.isAggressive = false;
    box.trackingID = 0;
    box.lastSeenTime = 0;
    box.isAlive = true;
    return box;
}

// ==========================================
//        MONSTER DATABASE IMPLEMENTATION
// ==========================================

void MonsterDatabase::LoadDefaultMonsters() {
    // Default RF Online monsters (example colors)
    MonsterInfo monster;
    
    // Bellato monsters (Blue-ish)
    monster.name = "Bellato_Accretian";
    monster.color = { 50, 100, 200, 45, "Blue" };
    monster.priority = 5;
    monster.aggressive = true;
    monster.hpBarOffset = 50;
    monster.estimatedHP = 1000;
    monster.expReward = 500;
    monster.ignore = false;
    monsters[monster.name] = monster;
    allColors.push_back(monster.color);
    
    // Accretian monsters (Red-ish)
    monster.name = "Accretian_Warrior";
    monster.color = { 200, 80, 80, 45, "Red" };
    monster.priority = 4;
    monster.aggressive = true;
    monster.hpBarOffset = 50;
    monster.estimatedHP = 1500;
    monster.expReward = 750;
    monster.ignore = false;
    monsters[monster.name] = monster;
    allColors.push_back(monster.color);
    
    // Cora monsters (Purple-ish)
    monster.name = "Cora_Summoner";
    monster.color = { 150, 80, 180, 45, "Purple" };
    monster.priority = 3;
    monster.aggressive = false;
    monster.hpBarOffset = 50;
    monster.estimatedHP = 800;
    monster.expReward = 400;
    monster.ignore = false;
    monsters[monster.name] = monster;
    allColors.push_back(monster.color);
    
    // Neutral mobs
    monster.name = "Neutral_Monster";
    monster.color = { 100, 150, 100, 45, "Green" };
    monster.priority = 7;
    monster.aggressive = false;
    monster.hpBarOffset = 50;
    monster.estimatedHP = 600;
    monster.expReward = 300;
    monster.ignore = false;
    monsters[monster.name] = monster;
    allColors.push_back(monster.color);
}

void MonsterDatabase::AddMonster(const MonsterInfo& monster) {
    monsters[monster.name] = monster;
    
    // Check if color already exists
    bool colorExists = false;
    for (const auto& c : allColors) {
        if (c.r == monster.color.r && c.g == monster.color.g && c.b == monster.color.b) {
            colorExists = true;
            break;
        }
    }
    if (!colorExists) {
        allColors.push_back(monster.color);
    }
}

void MonsterDatabase::RemoveMonster(const string& name) {
    auto it = monsters.find(name);
    if (it != monsters.end()) {
        monsters.erase(it);
    }
}

MonsterInfo* MonsterDatabase::GetMonster(const string& name) {
    auto it = monsters.find(name);
    if (it != monsters.end()) {
        return &it->second;
    }
    return nullptr;
}

vector<MonsterInfo> MonsterDatabase::GetAllMonsters() const {
    vector<MonsterInfo> result;
    for (const auto& pair : monsters) {
        result.push_back(pair.second);
    }
    return result;
}

vector<ColorRGB> MonsterDatabase::GetAllColors() const {
    return allColors;
}

void MonsterDatabase::LoadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        stringstream ss(line);
        MonsterInfo monster;
        string token;
        
        // Format: name,r,g,b,tolerance,priority,aggressive,hpOffset,exp,ignore
        getline(ss, monster.name, ',');
        getline(ss, token, ','); monster.color.r = stoi(token);
        getline(ss, token, ','); monster.color.g = stoi(token);
        getline(ss, token, ','); monster.color.b = stoi(token);
        getline(ss, token, ','); monster.color.tolerance = stoi(token);
        getline(ss, token, ','); monster.priority = stoi(token);
        getline(ss, token, ','); monster.aggressive = (token == "1");
        getline(ss, token, ','); monster.hpBarOffset = stoi(token);
        getline(ss, token, ','); monster.expReward = stoi(token);
        getline(ss, token, ','); monster.ignore = (token == "1");
        
        monster.color.name = monster.name;
        monster.estimatedHP = 1000;
        
        monsters[monster.name] = monster;
        allColors.push_back(monster.color);
    }
    
    file.close();
}

void MonsterDatabase::SaveToFile(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) return;

    file << "# Monster Database Format\n";
    file << "# name,r,g,b,tolerance,priority,aggressive,hpOffset,exp,ignore\n";

    for (const auto& pair : monsters) {
        const MonsterInfo& m = pair.second;
        file << m.name << ","
             << m.color.r << ","
             << m.color.g << ","
             << m.color.b << ","
             << m.color.tolerance << ","
             << m.priority << ","
             << (m.aggressive ? 1 : 0) << ","
             << m.hpBarOffset << ","
             << m.expReward << ","
             << (m.ignore ? 1 : 0) << "\n";
    }

    file.close();
}

// ==========================================
//        SKILL MANAGER IMPLEMENTATION
// ==========================================

SkillManager::SkillManager() : currentSkillIndex(0), rotationEnabled(true) {}

void SkillManager::AddSkill(const SkillInfo& skill) {
    skills.push_back(skill);
}

void SkillManager::RemoveSkill(int index) {
    if (index >= 0 && index < (int)skills.size()) {
        skills.erase(skills.begin() + index);
    }
}

SkillInfo* SkillManager::GetNextSkill(const PlayerStatus& player, const BoundingBox& target) {
    UpdateCooldowns();
    
    for (int i = 0; i < (int)skills.size(); i++) {
        int idx = (currentSkillIndex + i) % skills.size();
        SkillInfo& skill = skills[idx];
        
        if (!skill.enabled) continue;
        
        DWORD currentTime = GetTickCount();
        if (currentTime - skill.lastUsedTime < skill.cooldownMs) continue;
        
        // Check HP conditions
        double hpPercent = player.GetHPPercent();
        if (hpPercent < skill.minHPPercentToUse || hpPercent > skill.maxHPPercentToUse) continue;
        
        // Set as next skill
        currentSkillIndex = idx;
        skill.lastUsedTime = currentTime;
        return &skill;
    }
    
    return nullptr;
}

void SkillManager::ResetCooldowns() {
    for (auto& skill : skills) {
        skill.lastUsedTime = 0;
    }
}

void SkillManager::UpdateCooldowns() {
    // Cooldowns update automatically on GetNextSkill
}

void SkillManager::EnableSkill(int index, bool enabled) {
    if (index >= 0 && index < (int)skills.size()) {
        skills[index].enabled = enabled;
    }
}

vector<SkillInfo> SkillManager::GetAllSkills() const {
    return skills;
}

void SkillManager::LoadDefaultSkills() {
    skills.clear();
    
    // Attack skills
    SkillInfo skill;
    skill.name = "Basic Attack";
    skill.key = VK_SPACE;
    skill.cooldownMs = 1000;
    skill.priority = 10;
    skill.minHPPercentToUse = 0;
    skill.maxHPPercentToUse = 100;
    skill.lastUsedTime = 0;
    skill.enabled = true;
    skills.push_back(skill);
    
    skill.name = "Skill 1 (F1)";
    skill.key = VK_F1;
    skill.cooldownMs = 5000;
    skill.priority = 5;
    skills.push_back(skill);
    
    skill.name = "Skill 2 (F2)";
    skill.key = VK_F2;
    skill.cooldownMs = 8000;
    skill.priority = 4;
    skills.push_back(skill);
    
    skill.name = "Skill 3 (F3)";
    skill.key = VK_F3;
    skill.cooldownMs = 10000;
    skill.priority = 3;
    skills.push_back(skill);
    
    skill.name = "Skill 4 (F4)";
    skill.key = VK_F4;
    skill.cooldownMs = 12000;
    skill.priority = 2;
    skills.push_back(skill);
    
    // Buff skills
    skill.name = "Buff 1 (F5)";
    skill.key = VK_F5;
    skill.cooldownMs = 30000;
    skill.priority = 1;
    skill.enabled = false;
    skills.push_back(skill);
    
    skill.name = "Buff 2 (F6)";
    skill.key = VK_F6;
    skill.cooldownMs = 60000;
    skill.priority = 1;
    skill.enabled = false;
    skills.push_back(skill);
}

// ==========================================
//        CONFIGURATION MANAGER IMPLEMENTATION
// ==========================================

ConfigManager::ConfigManager() : configPath("monsterfinder_config.ini") {
    ResetToDefaults();
}

void ConfigManager::ResetToDefaults() {
    // Scanning
    config.scanWidth = 1400;
    config.scanHeight = 700;
    config.scanStep = 3;
    config.playerSafeRadius = 130;
    config.motionThreshold = 30;
    config.minBoxSize = 15;
    config.mergeDistance = 60;
    
    // Attack
    config.maxAttackTime = 10000;
    config.attackCooldownMs = 500;
    config.autoSkill = true;
    config.attackDelayMs = 100;
    config.chainAttack = true;
    
    // Targeting
    config.targetMode = "priority";
    config.onlyAggressive = false;
    config.avoidElite = true;
    
    // Movement
    config.autoLoot = true;
    config.autoLootKey = 'X';
    config.autoLootIntervalMs = 2000;
    config.avoidPlayers = true;
    config.playerAvoidDistance = 200;
    
    // Safety
    config.playerHPThreshold = 30;
    config.escapeKey = VK_ESCAPE;
    config.autoEscape = true;
    config.pauseOnLowHP = true;
    
    // Advanced
    config.randomDelay = true;
    config.randomDelayMin = 50;
    config.randomDelayMax = 200;
    config.humanLikeBehavior = true;
    config.clickDeviation = 5;
    config.clickDelayMin = 100;
    config.clickDelayMax = 300;
    
    // Display
    config.showBoxes = true;
    config.showOverlay = true;
    config.showDebugInfo = false;
    config.showESPLines = true;      // Enable ESP lines by default
    config.enablePathfinding = true; // Enable pathfinding by default
    config.maxESPLines = 10;         // Show max 10 nearest monsters
}

void ConfigManager::LoadConfig(const string& path) {
    string filePath = path.empty() ? configPath : path;
    
    // Simple INI parsing (can be enhanced)
    ifstream file(filePath);
    if (!file.is_open()) {
        return; // Use defaults
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '[') continue;
        
        size_t pos = line.find('=');
        if (pos == string::npos) continue;
        
        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);
        
        // Parse simple key-value pairs
        if (key == "scanWidth") config.scanWidth = stoi(value);
        else if (key == "scanHeight") config.scanHeight = stoi(value);
        else if (key == "autoLoot") config.autoLoot = (value == "1");
        else if (key == "autoLootIntervalMs") config.autoLootIntervalMs = stoi(value);
        // ... add more config parsing as needed
    }
    
    file.close();
}

void ConfigManager::SaveConfig(const string& path) {
    string filePath = path.empty() ? configPath : path;
    
    ofstream file(filePath);
    if (!file.is_open()) return;

    file << "[Scanning]\n";
    file << "scanWidth=" << config.scanWidth << "\n";
    file << "scanHeight=" << config.scanHeight << "\n";
    file << "scanStep=" << config.scanStep << "\n";
    file << "playerSafeRadius=" << config.playerSafeRadius << "\n";
    file << "motionThreshold=" << config.motionThreshold << "\n";
    file << "\n[Attack]\n";
    file << "maxAttackTime=" << config.maxAttackTime << "\n";
    file << "autoSkill=" << (config.autoSkill ? 1 : 0) << "\n";
    file << "\n[Movement]\n";
    file << "autoLoot=" << (config.autoLoot ? 1 : 0) << "\n";
    file << "autoLootIntervalMs=" << config.autoLootIntervalMs << "\n";
    file << "\n[Safety]\n";
    file << "playerHPThreshold=" << config.playerHPThreshold << "\n";
    file << "autoEscape=" << (config.autoEscape ? 1 : 0) << "\n";
    
    file.close();
}

// ==========================================
//        STATISTICS MANAGER IMPLEMENTATION
// ==========================================

StatsManager::StatsManager() {
    stats.totalKills = 0;
    stats.totalExp = 0;
    stats.itemsLooted = 0;
    stats.potionsUsed = 0;
    stats.startTime = 0;
    stats.runningTime = 0;
    stats.monstersPerMinute = 0;
    stats.expPerMinute = 0;
}

void StatsManager::StartSession() {
    stats.startTime = GetTickCount();
    stats.totalKills = 0;
    stats.totalExp = 0;
    stats.itemsLooted = 0;
    stats.potionsUsed = 0;
}

void StatsManager::EndSession() {
    if (stats.startTime > 0) {
        stats.runningTime = GetTickCount() - stats.startTime;
    }
}

void StatsManager::RecordKill(const MonsterInfo& monster) {
    stats.totalKills++;
    stats.totalExp += monster.expReward;
    UpdateStats();
}

void StatsManager::RecordLoot(int items) {
    stats.itemsLooted += items;
}

void StatsManager::RecordPotionUsed() {
    stats.potionsUsed++;
}

void StatsManager::UpdateStats() {
    if (stats.startTime == 0) return;
    
    DWORD runningMs = GetTickCount() - stats.startTime;
    double runningMinutes = runningMs / 60000.0;
    
    if (runningMinutes > 0) {
        stats.monstersPerMinute = (int)(stats.totalKills / runningMinutes);
        stats.expPerMinute = stats.totalExp / runningMinutes;
    }
}

Statistics StatsManager::GetStats() const {
    return stats;
}

string StatsManager::GetFormattedStats() const {
    stringstream ss;
    DWORD runningMs = stats.startTime > 0 ? (GetTickCount() - stats.startTime) : stats.runningTime;
    int runningSeconds = runningMs / 1000;
    int minutes = runningSeconds / 60;
    int seconds = runningSeconds % 60;
    
    ss << "=== Statistics ===\n";
    ss << "Running Time: " << minutes << "m " << seconds << "s\n";
    ss << "Total Kills: " << stats.totalKills << "\n";
    ss << "Total EXP: " << stats.totalExp << "\n";
    ss << "Items Looted: " << stats.itemsLooted << "\n";
    ss << "Potions Used: " << stats.potionsUsed << "\n";
    ss << "Kills/Min: " << stats.monstersPerMinute << "\n";
    ss << "EXP/Min: " << fixed << setprecision(1) << stats.expPerMinute << "\n";
    
    return ss.str();
}

void StatsManager::SaveToFile(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) return;

    file << "[Session Statistics]\n";
    file << "RunningTime=" << stats.runningTime << "\n";
    file << "TotalKills=" << stats.totalKills << "\n";
    file << "TotalEXP=" << stats.totalExp << "\n";
    file << "ItemsLooted=" << stats.itemsLooted << "\n";
    file << "PotionsUsed=" << stats.potionsUsed << "\n";
    file << "MonstersPerMinute=" << stats.monstersPerMinute << "\n";
    file << "EXPPerMinute=" << stats.expPerMinute << "\n";
    
    file.close();
}

// ==========================================
//        LOGGER IMPLEMENTATION
// ==========================================

Logger::Logger() : consoleOutput(true) {}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::Init(const string& path, bool console) {
    consoleOutput = console;
    logPath = path;
    
    time_t now = time(0);
    tm* ltm = localtime(&now);
    
    char filename[256];
    sprintf(filename, "%s_%04d%02d%02d_%02d%02d%02d.log", 
            path.c_str(),
            ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
            ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    
    logFile.open(filename, ios::out | ios::app);
}

string Logger::GetTimestamp() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    
    char buffer[64];
    sprintf(buffer, "[%04d-%02d-%02d %02d:%02d:%02d]",
            ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
            ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    
    return string(buffer);
}

string Logger::LevelToString(LogLevel level) {
    switch (level) {
        case LOG_INFO: return "[INFO]";
        case LOG_WARNING: return "[WARN]";
        case LOG_ERROR: return "[ERROR]";
        case LOG_DEBUG: return "[DEBUG]";
        default: return "[UNKNOWN]";
    }
}

void Logger::Log(LogLevel level, const string& message) {
    string logLine = GetTimestamp() + " " + LevelToString(level) + " " + message + "\n";
    
    if (consoleOutput) {
        if (level == LOG_ERROR) {
            cerr << logLine;
        } else {
            cout << logLine;
        }
    }
    
    if (logFile.is_open()) {
        logFile << logLine;
        logFile.flush();
    }
}

void Logger::Info(const string& message) { Log(LOG_INFO, message); }
void Logger::Warning(const string& message) { Log(LOG_WARNING, message); }
void Logger::Error(const string& message) { Log(LOG_ERROR, message); }
void Logger::Debug(const string& message) { Log(LOG_DEBUG, message); }

// ==========================================
//        INPUT MANAGER IMPLEMENTATION
// ==========================================

InputManager::InputManager(HWND hwnd) 
    : targetWindow(hwnd), clickDeviation(5), clickDelayMin(100), clickDelayMax(300), randomDelay(true) {}

void InputManager::SetClickDeviation(int deviation) { clickDeviation = deviation; }

void InputManager::SetClickDelay(int minMs, int maxMs) {
    clickDelayMin = minMs;
    clickDelayMax = maxMs;
}

void InputManager::SetRandomDelay(bool enabled) { randomDelay = enabled; }

int InputManager::GetRandomClickDelay() {
    if (randomDelay) {
        return clickDelayMin + rand() % (clickDelayMax - clickDelayMin);
    }
    return clickDelayMin;
}

int InputManager::GetRandomDeviation() {
    if (randomDelay && clickDeviation > 0) {
        return (rand() % (clickDeviation * 2 + 1)) - clickDeviation;
    }
    return 0;
}

void InputManager::Click(int x, int y) {
    int deviation = GetRandomDeviation();
    int targetX = x + deviation;
    int targetY = y + deviation;
    
    POINT pt = { targetX, targetY };
    ClientToScreen(targetWindow, &pt);
    
    INPUT inputs[4] = {};
    
    // Move mouse
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dx = (pt.x * 65535) / GetSystemMetrics(SM_CXSCREEN);
    inputs[0].mi.dy = (pt.y * 65535) / GetSystemMetrics(SM_CYSCREEN);
    inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    
    // Mouse down
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    
    // Mouse up
    inputs[2].type = INPUT_MOUSE;
    inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    
    SendInput(3, inputs, sizeof(INPUT));
    
    Sleep(GetRandomClickDelay());
}

void InputManager::KeyPress(WORD key) {
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = key;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = key;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, inputs, sizeof(INPUT));
}

void InputManager::KeyDown(WORD key) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    SendInput(1, &input, sizeof(INPUT));
}

void InputManager::KeyUp(WORD key) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void InputManager::SendString(const string& text) {
    for (char c : text) {
        SHORT vk = VkKeyScanA(c);
        WORD key = vk & 0xFF;
        KeyPress(key);
        Sleep(50);
    }
}

// ==========================================
//        HOTKEY MANAGER IMPLEMENTATION
// ==========================================

HotkeyManager::HotkeyManager() : enabled(true) {}

void HotkeyManager::RegisterHotkey(WORD key, HotkeyAction action, const string& description) {
    hotkeys[key] = { key, action, true, description };
}

void HotkeyManager::UnregisterHotkey(WORD key) {
    hotkeys.erase(key);
}

void HotkeyManager::EnableHotkey(WORD key, bool enable) {
    auto it = hotkeys.find(key);
    if (it != hotkeys.end()) {
        it->second.enabled = enable;
    }
}

void HotkeyManager::SetAllEnabled(bool enabled) {
    this->enabled = enabled;
    for (auto& pair : hotkeys) {
        pair.second.enabled = enabled;
    }
}

vector<HotkeyAction> HotkeyManager::CheckHotkeys() {
    vector<HotkeyAction> actions;
    
    if (!enabled) return actions;
    
    for (const auto& pair : hotkeys) {
        if (GetAsyncKeyState(pair.first) & 0x8000) {
            if (pair.second.enabled) {
                actions.push_back(pair.second.action);
            }
        }
    }
    
    return actions;
}

Hotkey* HotkeyManager::GetHotkey(WORD key) {
    auto it = hotkeys.find(key);
    if (it != hotkeys.end()) {
        return &it->second;
    }
    return nullptr;
}

vector<Hotkey> HotkeyManager::GetAllHotkeys() const {
    vector<Hotkey> result;
    for (const auto& pair : hotkeys) {
        result.push_back(pair.second);
    }
    return result;
}

void HotkeyManager::LoadDefaultHotkeys() {
    hotkeys.clear();
    
    RegisterHotkey(VK_F5, HK_START_BOT, "Start Bot");
    RegisterHotkey(VK_F6, HK_STOP_BOT, "Stop Bot");
    RegisterHotkey(VK_F7, HK_PAUSE_BOT, "Pause/Resume Bot");
    RegisterHotkey(VK_F8, HK_TOGGLE_AUTO_LOOT, "Toggle Auto Loot");
    RegisterHotkey(VK_F9, HK_TOGGLE_SHOW_BOXES, "Toggle Show Boxes");
    RegisterHotkey(VK_F10, HK_ESCAPE, "Emergency Escape");
    RegisterHotkey(VK_F11, HK_RELOAD_CONFIG, "Reload Configuration");
    RegisterHotkey(VK_F12, HK_QUICK_TARGET, "Quick Target");
}

// ==========================================
//        MONSTER FINDER BOT IMPLEMENTATION
// ==========================================

MonsterFinderBot::MonsterFinderBot() {
    scanner = nullptr;
    isRunning = false;
    isPaused = false;
    isAttacking = false;
    attackStartTime = 0;
    lastLootTime = 0;
    lastUpdateTime = 0;
    lastScanTime = 0;
    nextTrackingID = 1;
    detectedBoxes.clear();
    trackedMonsters.clear();
}

MonsterFinderBot::~MonsterFinderBot() {
    Shutdown();
    
    if (inputManager) {
        delete inputManager;
        inputManager = nullptr;
    }
    
    if (scanner) {
        delete scanner;
        scanner = nullptr;
    }
}

bool MonsterFinderBot::Initialize(HWND hwnd) {
    gameWindow = hwnd;
    
    if (!hwnd) {
        logger.Error("Invalid window handle");
        return false;
    }
    
    // Initialize components
    config = configManager.GetConfig();
    scanner = new EnhancedScanner(hwnd, config.scanWidth, config.scanHeight);
    inputManager = new InputManager(hwnd);
    
    // Load data
    monsterDB.LoadDefaultMonsters();
    skillManager.LoadDefaultSkills();
    hotkeyManager.LoadDefaultHotkeys();
    
    // Setup potion config
    potionConfig.hpKey = VK_NUMPAD1;
    potionConfig.mpKey = VK_NUMPAD2;
    potionConfig.hpPercentThreshold = config.playerHPThreshold;
    potionConfig.mpPercentThreshold = 20;
    potionConfig.useDelayMs = 1000;
    potionConfig.lastUseTime = 0;
    potionConfig.enabled = true;
    
    // Initialize player status
    playerStatus.currentHP = 1000;
    playerStatus.maxHP = 1000;
    playerStatus.currentMP = 500;
    playerStatus.maxMP = 500;
    playerStatus.level = 1;
    playerStatus.currentExp = 0;
    playerStatus.expToNext = 1000;
    
    logger.Init("monsterfinder");
    logger.Info("MonsterFinderBot initialized");
    
    return true;
}

bool MonsterFinderBot::LoadAllConfigurations() {
    configManager.LoadConfig();
    config = configManager.GetConfig();
    
    monsterDB.LoadFromFile("monsters.ini");
    monsterDB.SaveToFile("monsters_backup.ini");
    
    statsManager.StartSession();
    
    return true;
}

void MonsterFinderBot::Shutdown() {
    Stop();
    
    if (scanner) {
        delete scanner;
        scanner = nullptr;
    }
    
    statsManager.EndSession();
    statsManager.SaveToFile("stats.ini");
    logger.Info("MonsterFinderBot shutdown");
}

void MonsterFinderBot::Start() {
    if (isRunning) return;
    
    isRunning = true;
    isPaused = false;
    statsManager.StartSession();
    
    botThread = thread(&MonsterFinderBot::BotLoop, this);
    logger.Info("Bot started");
}

void MonsterFinderBot::Stop() {
    if (!isRunning) return;
    
    isRunning = false;
    isPaused = false;
    isAttacking = false;
    
    if (botThread.joinable()) {
        botThread.join();
    }
    
    statsManager.EndSession();
    logger.Info("Bot stopped");
}

void MonsterFinderBot::Pause() {
    if (!isRunning) return;
    isPaused = true;
    logger.Info("Bot paused");
}

void MonsterFinderBot::Resume() {
    if (!isRunning) return;
    isPaused = false;
    logger.Info("Bot resumed");
}

void MonsterFinderBot::BotLoop() {
    while (isRunning) {
        if (!isPaused) {
            // Check hotkeys
            CheckHotkeys();
            
            // Update player status
            UpdatePlayerStatus();
            
            // Safety check
            if (config.pauseOnLowHP && playerStatus.IsCriticalHP()) {
                Pause();
                logger.Warning("Critical HP! Bot paused.");
                continue;
            }
            
            // Use potions if needed
            UsePotions();
            
            // Auto loot
            if (config.autoLoot) {
                AutoLoot();
            }
            
            // Always scan to update tracked monsters (even when attacking)
            // This ensures boxes follow monster movement
            DWORD currentTime = GetTickCount();
            // Scan more frequently when not attacking to find new targets faster
            DWORD scanInterval = isAttacking ? 200 : 100;  // 200ms when attacking, 100ms when idle
            if (currentTime - lastScanTime > scanInterval || lastScanTime == 0) {
                scanner->Capture();
                
                // Find all targets
                vector<ColorRGB> colors = monsterDB.GetAllColors();
                vector<TargetPos> rawPixels = scanner->FindColorTargets(colors, true);
                
                // Group pixels into bounding boxes
                map<string, vector<BoundingBox>> boxesByColor;
                
                for (const auto& p : rawPixels) {
                    bool merged = false;
                    for (auto& box : boxesByColor[p.color.name]) {
                        double distance = sqrt(pow(p.x - box.centerX, 2.0) + pow(p.y - box.centerY, 2.0));
                        if (distance < config.mergeDistance) {
                            if (p.x < box.minX) box.minX = p.x;
                            if (p.x > box.maxX) box.maxX = p.x;
                            if (p.y < box.minY) box.minY = p.y;
                            if (p.y > box.maxY) box.maxY = p.y;
                            box.pixelCount++;
                            box.centerX = (box.minX + box.maxX) / 2;
                            box.centerY = (box.minY + box.maxY) / 2;
                            box.color = p.color;
                            merged = true;
                            break;
                        }
                    }
                    
                    if (!merged) {
                        BoundingBox box;
                        box.minX = p.x; box.maxX = p.x;
                        box.minY = p.y; box.maxY = p.y;
                        box.pixelCount = 1;
                        box.centerX = p.x;
                        box.centerY = p.y;
                        box.color = p.color;
                        box.priority = 5;
                        box.isAggressive = false;
                        box.trackingID = 0;
                        box.lastSeenTime = 0;
                        box.isAlive = true;
                        boxesByColor[p.color.name].push_back(box);
                    }
                }
                
                // Filter boxes
                vector<BoundingBox> validBoxes;
                for (auto& colorPair : boxesByColor) {
                    for (auto& box : colorPair.second) {
                        if (box.pixelCount < config.minBoxSize) continue;
                        box.distToCenter = sqrt(pow(box.centerX - config.scanWidth / 2.0, 2.0) + 
                                                  pow(box.centerY - config.scanHeight / 2.0, 2.0));
                        if (box.distToCenter < config.playerSafeRadius) continue;
                        
                        for (const auto& monsterPair : monsterDB.GetAllMonsters()) {
                            if (monsterPair.color.name == box.color.name) {
                                box.monsterName = monsterPair.name;
                                box.priority = monsterPair.priority;
                                box.isAggressive = monsterPair.aggressive;
                                box.estimatedHP = monsterPair.estimatedHP;
                                break;
                            }
                        }
                        
                        if (config.onlyAggressive && !box.isAggressive) continue;
                        validBoxes.push_back(box);
                    }
                }
                
                // Update tracked monsters
                UpdateTrackedMonsters(validBoxes);
                
                // Remove dead monsters from tracking
                RemoveDeadMonsters();
                
                // Update detectedBoxes for display (only alive monsters)
                detectedBoxes.clear();
                for (const auto& pair : trackedMonsters) {
                    if (pair.second.isAlive) {
                        detectedBoxes.push_back(pair.second);
                    }
                }
                
                lastScanTime = currentTime;
            }
            
            // Main bot logic
            if (isAttacking) {
                AttackTarget();
            } else {
                // When not attacking, always try to find new target
                ScanAndTarget();
            }
            
            // Draw overlay (always draw to show all tracked monsters)
            if (config.showOverlay) {
                DrawOverlay();
            }
            
            lastUpdateTime = GetTickCount();
        }
        
        Sleep(10);
    }
}

int MonsterFinderBot::MatchMonsterToTracked(const BoundingBox& newBox) {
    // Match new detection with existing tracked monsters
    // Based on position proximity and color similarity
    int bestMatchID = -1;
    double bestDistance = 1000.0;  // Max distance for matching
    
    for (auto& pair : trackedMonsters) {
        const BoundingBox& tracked = pair.second;
        
        // Check if same color
        if (tracked.color.name != newBox.color.name) continue;
        
        // Calculate distance between centers
        double distance = sqrt(pow(tracked.centerX - newBox.centerX, 2.0) + 
                              pow(tracked.centerY - newBox.centerY, 2.0));
        
        // Match if within reasonable distance (monsters don't move too fast)
        if (distance < bestDistance && distance < 100.0) {
            bestDistance = distance;
            bestMatchID = pair.first;
        }
    }
    
    return bestMatchID;
}

void MonsterFinderBot::UpdateTrackedMonsters(const vector<BoundingBox>& detectedBoxes) {
    DWORD currentTime = GetTickCount();
    set<int> matchedIDs;
    
    // Update or add tracked monsters
    for (const auto& newBox : detectedBoxes) {
        int matchedID = MatchMonsterToTracked(newBox);
        
        if (matchedID > 0) {
            // Update existing tracked monster
            BoundingBox& tracked = trackedMonsters[matchedID];
            tracked = newBox;
            tracked.trackingID = matchedID;
            tracked.lastSeenTime = currentTime;
            tracked.isAlive = IsMonsterAlive(tracked);
            matchedIDs.insert(matchedID);
        } else {
            // New monster detected, assign new tracking ID
            BoundingBox tracked = newBox;
            tracked.trackingID = nextTrackingID++;
            tracked.lastSeenTime = currentTime;
            tracked.isAlive = IsMonsterAlive(tracked);
            trackedMonsters[tracked.trackingID] = tracked;
            matchedIDs.insert(tracked.trackingID);
        }
    }
    
    // Remove monsters that are no longer detected or are dead
    auto it = trackedMonsters.begin();
    while (it != trackedMonsters.end()) {
        if (matchedIDs.find(it->first) == matchedIDs.end()) {
            // Not matched in this scan, check if should be removed
            DWORD timeSinceLastSeen = currentTime - it->second.lastSeenTime;
            if (timeSinceLastSeen > 2000) {  // Remove if not seen for 2 seconds
                it = trackedMonsters.erase(it);
            } else {
                ++it;
            }
        } else {
            // Check if monster is dead
            if (!it->second.isAlive) {
                it = trackedMonsters.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void MonsterFinderBot::RemoveDeadMonsters() {
    // Remove tracked monsters that are confirmed dead
    auto it = trackedMonsters.begin();
    while (it != trackedMonsters.end()) {
        if (!IsMonsterAlive(it->second)) {
            it->second.isAlive = false;
            // Remove immediately if dead
            it = trackedMonsters.erase(it);
        } else {
            it->second.isAlive = true;
            ++it;
        }
    }
}

void MonsterFinderBot::ScanAndTarget() {
    // Wait a bit for scan to complete if needed
    // Select target from tracked monsters
    if (!detectedBoxes.empty()) {
        // Filter out dead monsters
        vector<BoundingBox> aliveBoxes;
        for (const auto& box : detectedBoxes) {
            if (box.isAlive && box.trackingID > 0) {
                // Verify it's still in tracked monsters
                auto it = trackedMonsters.find(box.trackingID);
                if (it != trackedMonsters.end() && it->second.isAlive) {
                    aliveBoxes.push_back(box);
                }
            }
        }
        
        if (!aliveBoxes.empty()) {
            BoundingBox target = SelectTarget(aliveBoxes);
            
            // Find the tracked version to get tracking ID
            auto it = trackedMonsters.find(target.trackingID);
            if (it != trackedMonsters.end()) {
                target = it->second;
            } else {
                // Try to find by position
                for (auto& pair : trackedMonsters) {
                    if (abs(pair.second.centerX - target.centerX) < 10 && 
                        abs(pair.second.centerY - target.centerY) < 10) {
                        target = pair.second;
                        break;
                    }
                }
            }
            
            // Focus game window
            if (GetForegroundWindow() != gameWindow) {
                ShowWindow(gameWindow, SW_RESTORE);
                SetForegroundWindow(gameWindow);
                Sleep(50);
            }
            
            // Get click position
            RECT rect;
            GetClientRect(gameWindow, &rect);
            int offsetX = (rect.right - rect.left) / 2 - config.scanWidth / 2;
            int offsetY = (rect.bottom - rect.top) / 2 - config.scanHeight / 2;
            
            int clickX = offsetX + target.centerX;
            int clickY = offsetY + target.centerY;
            
            // Follow ESP path jika enabled
            if (config.enablePathfinding) {
                FollowESPPath(target);
            } else {
                // Direct click
                inputManager->Click(clickX, clickY);
            }
            
            Sleep(ApplyRandomDelay(config.attackDelayMs));
            
            // Attack
            inputManager->KeyPress(VK_SPACE);
            
            currentTarget = target;
            isAttacking = true;
            attackStartTime = GetTickCount();
            
            logger.Info("Targeting: " + target.monsterName + " (ID: " + to_string(target.trackingID) + ")");
        } else {
            // No alive monsters found, wait for scan
            logger.Debug("No alive monsters found, waiting for scan...");
        }
    } else {
        // No detected boxes, wait for scan
        logger.Debug("No monsters detected, scanning...");
    }
}

void MonsterFinderBot::AttackTarget() {
    DWORD currentTime = GetTickCount();
    
    // Check for timeout
    if (currentTime - attackStartTime > config.maxAttackTime) {
        logger.Warning("Attack timeout! Skipping target.");
        inputManager->KeyPress(config.escapeKey);
        isAttacking = false;
        return;
    }
    
    // Check if monster is still alive
    if (!IsMonsterAlive(currentTarget)) {
        logger.Info("Target defeated: " + currentTarget.monsterName + " (ID: " + to_string(currentTarget.trackingID) + ")");
        
        // Record kill
        MonsterInfo* monster = monsterDB.GetMonster(currentTarget.monsterName);
        if (monster) {
            statsManager.RecordKill(*monster);
        }
        
        // Remove dead monster from tracking (box will disappear)
        trackedMonsters.erase(currentTarget.trackingID);
        
        // Also remove from detectedBoxes
        detectedBoxes.erase(
            remove_if(detectedBoxes.begin(), detectedBoxes.end(),
                [this](const BoundingBox& box) {
                    return box.trackingID == currentTarget.trackingID;
                }),
            detectedBoxes.end()
        );
        
        // Loot the dead monster
        if (config.autoLoot) {
            logger.Info("Looting dead monster...");
            inputManager->KeyPress(config.autoLootKey);
            Sleep(500); // Delay for loot animation
            // Try looting multiple times to ensure items are picked up
            for (int i = 0; i < 3; i++) {
                inputManager->KeyPress(config.autoLootKey);
                Sleep(300);
            }
            lastLootTime = GetTickCount(); // Update loot time
        }
        
        // Stop attacking and clear target
        isAttacking = false;
        memset(&currentTarget, 0, sizeof(BoundingBox)); // Clear current target
        currentTarget.isAlive = true;
        
        // Force immediate scan for new targets
        lastScanTime = 0; // Reset scan time to force immediate scan
        
        logger.Info("Searching for new targets...");
        return;
    }
    
    // Update current target's alive status in tracked monsters
    if (currentTarget.trackingID > 0) {
        auto it = trackedMonsters.find(currentTarget.trackingID);
        if (it != trackedMonsters.end()) {
            it->second.isAlive = IsMonsterAlive(currentTarget);
            // Update current target position from tracked version
            currentTarget = it->second;
        }
    }
    
    // Execute skills
    if (config.autoSkill) {
        ExecuteSkillRotation();
    }
    
    // Keep attacking
    if (config.chainAttack) {
        inputManager->KeyPress(VK_SPACE);
        Sleep(ApplyRandomDelay(config.attackCooldownMs));
    }
}

void MonsterFinderBot::CheckPlayerHP() {
    // This would read HP from the game UI
    // For now, we use simulated values
}

void MonsterFinderBot::UsePotions() {
    if (!potionConfig.enabled) return;
    
    DWORD currentTime = GetTickCount();
    if (currentTime - potionConfig.lastUseTime < potionConfig.useDelayMs) return;
    
    double hpPercent = playerStatus.GetHPPercent();
    double mpPercent = playerStatus.GetMPPercent();
    
    if (hpPercent < potionConfig.hpPercentThreshold) {
        inputManager->KeyPress(potionConfig.hpKey);
        potionConfig.lastUseTime = currentTime;
        statsManager.RecordPotionUsed();
        logger.Info("Used HP Potion");
    }
    else if (mpPercent < potionConfig.mpPercentThreshold) {
        inputManager->KeyPress(potionConfig.mpKey);
        potionConfig.lastUseTime = currentTime;
        logger.Info("Used MP Potion");
    }
}

void MonsterFinderBot::AutoLoot() {
    DWORD currentTime = GetTickCount();
    if (currentTime - lastLootTime < config.autoLootIntervalMs) return;
    
    inputManager->KeyPress(config.autoLootKey);
    lastLootTime = currentTime;
}

BoundingBox MonsterFinderBot::SelectTarget(const vector<BoundingBox>& candidates) {
    if (candidates.empty()) {
        BoundingBox empty;
        memset(&empty, 0, sizeof(BoundingBox));
        empty.isAlive = true;
        return empty;
    }
    
    // Sort berdasarkan target mode dengan prioritas jarak terdekat
    vector<BoundingBox> sorted = candidates;
    
    if (config.targetMode == "nearest" || config.enablePathfinding) {
        // Prioritaskan monster terdekat untuk ESP line dan pathfinding
        sort(sorted.begin(), sorted.end(), 
             [](const BoundingBox& a, const BoundingBox& b) {
                 return a.distToCenter < b.distToCenter;
             });
    } else if (config.targetMode == "priority") {
        sort(sorted.begin(), sorted.end(), 
             [](const BoundingBox& a, const BoundingBox& b) {
                 if (a.priority != b.priority) {
                     return a.priority < b.priority;
                 }
                 // Jika priority sama, pilih yang terdekat
                 return a.distToCenter < b.distToCenter;
             });
    } else if (config.targetMode == "weakest") {
        sort(sorted.begin(), sorted.end(), 
             [](const BoundingBox& a, const BoundingBox& b) {
                 if (a.estimatedHP != b.estimatedHP) {
                     return a.estimatedHP < b.estimatedHP;
                 }
                 return a.distToCenter < b.distToCenter;
             });
    } else if (config.targetMode == "strongest") {
        sort(sorted.begin(), sorted.end(), 
             [](const BoundingBox& a, const BoundingBox& b) {
                 if (a.estimatedHP != b.estimatedHP) {
                     return a.estimatedHP > b.estimatedHP;
                 }
                 return a.distToCenter < b.distToCenter;
             });
    }
    
    return sorted[0];
}

bool MonsterFinderBot::IsMonsterAlive(const BoundingBox& box) {
    int r, g, b;
    int redPixelCount = 0;
    
    // Check HP bar above monster
    int scanTop = box.minY - 80;
    int scanBottom = box.minY - 10;
    
    if (scanTop < 0) scanTop = 0;
    
    for (int y = scanTop; y < scanBottom; y += 2) {
        for (int x = box.minX - 20; x < box.maxX + 20; x += 2) {
            if (scanner->GetPixel(x, y, r, g, b)) {
                if (r > 180 && g < 100 && b < 100) {
                    redPixelCount++;
                }
            }
        }
    }
    
    return redPixelCount > 15;
}

bool MonsterFinderBot::IsPlayerInSafeRange(const BoundingBox& box) {
    return box.distToCenter < config.playerSafeRadius;
}

void MonsterFinderBot::ExecuteSkillRotation() {
    SkillInfo* skill = skillManager.GetNextSkill(playerStatus, currentTarget);
    if (skill) {
        inputManager->KeyPress(skill->key);
        logger.Debug("Used skill: " + skill->name);
    }
}

void MonsterFinderBot::UpdatePlayerStatus() {
    // This would read player status from game UI
    // For now, we use simulated values that decrease over time
}

void MonsterFinderBot::DrawBoundingBox(HDC hdc, const BoundingBox& box, COLORREF color, int offsetX, int offsetY) {
    HPEN hPen = CreatePen(PS_SOLID, 2, color);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    Rectangle(hdc, offsetX + box.minX, offsetY + box.minY, 
              offsetX + box.maxX, offsetY + box.maxY);
    
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

vector<BoundingBox> MonsterFinderBot::SortMonstersByDistance(const vector<BoundingBox>& monsters) {
    // Sort monsters by distance to player (center of screen)
    vector<BoundingBox> sorted = monsters;
    
    sort(sorted.begin(), sorted.end(),
        [this](const BoundingBox& a, const BoundingBox& b) {
            return a.distToCenter < b.distToCenter;
        });
    
    return sorted;
}

COLORREF MonsterFinderBot::GetESPLineColor(const BoundingBox& monster, int index) {
    // Color berdasarkan priority dan jarak
    // Nearest = Bright Green, Medium = Yellow, Far = Red
    
    if (index == 0) {
        return RGB(0, 255, 0);  // Bright green for nearest
    } else if (index < 3) {
        return RGB(255, 255, 0);  // Yellow for top 3
    } else if (index < 5) {
        return RGB(255, 165, 0);  // Orange for top 5
    } else {
        return RGB(255, 0, 0);  // Red for others
    }
}

vector<POINT> MonsterFinderBot::CalculatePathPoints(int startX, int startY, int endX, int endY) {
    // Calculate path points following ESP line
    // Simple linear interpolation dengan beberapa waypoints
    vector<POINT> path;
    
    int steps = max(5, (int)sqrt(pow(endX - startX, 2) + pow(endY - startY, 2)) / 20);
    
    for (int i = 0; i <= steps; i++) {
        double t = (double)i / steps;
        POINT pt;
        pt.x = (int)(startX + (endX - startX) * t);
        pt.y = (int)(startY + (endY - startY) * t);
        path.push_back(pt);
    }
    
    return path;
}

void MonsterFinderBot::FollowESPPath(const BoundingBox& target) {
    // Follow ESP line path untuk attack monster
    if (!config.enablePathfinding) {
        return;
    }
    
    RECT rect;
    GetClientRect(gameWindow, &rect);
    int offsetX = (rect.right - rect.left) / 2 - config.scanWidth / 2;
    int offsetY = (rect.bottom - rect.top) / 2 - config.scanHeight / 2;
    
    // Player position (center of screen)
    int playerX = config.scanWidth / 2;
    int playerY = config.scanHeight / 2;
    
    // Target position
    int targetX = target.centerX;
    int targetY = target.centerY;
    
    // Calculate path points
    vector<POINT> path = CalculatePathPoints(playerX, playerY, targetX, targetY);
    
    // Follow path dengan beberapa waypoints (untuk pathfinding yang lebih smooth)
    // Skip beberapa waypoint untuk tidak terlalu lambat
    int stepSize = max(1, (int)path.size() / 5);  // Max 5 waypoints
    
    for (size_t i = stepSize; i < path.size(); i += stepSize) {
        POINT waypoint = path[i];
        
        // Convert to screen coordinates
        int screenX = offsetX + waypoint.x;
        int screenY = offsetY + waypoint.y;
        
        // Small movement towards waypoint (optional - bisa di-disable untuk langsung ke target)
        // inputManager->Click(screenX, screenY);
        // Sleep(50);
    }
    
    // Final click pada target
    int finalX = offsetX + targetX;
    int finalY = offsetY + targetY;
    inputManager->Click(finalX, finalY);
}

void MonsterFinderBot::DrawESPLines(HDC hdc, int offsetX, int offsetY) {
    if (!config.showESPLines) return;
    
    // Player position (center of screen)
    int playerX = config.scanWidth / 2;
    int playerY = config.scanHeight / 2;
    
    // Get all alive monsters sorted by distance
    vector<BoundingBox> aliveMonsters;
    for (const auto& pair : trackedMonsters) {
        if (pair.second.isAlive) {
            aliveMonsters.push_back(pair.second);
        }
    }
    
    // Sort by distance (nearest first)
    vector<BoundingBox> sortedMonsters = SortMonstersByDistance(aliveMonsters);
    
    // Draw ESP lines untuk maxESPLines terdekat
    int lineCount = min(config.maxESPLines, (int)sortedMonsters.size());
    
    for (int i = 0; i < lineCount; i++) {
        const BoundingBox& monster = sortedMonsters[i];
        
        // Skip current target (akan digambar terpisah)
        if (isAttacking && monster.trackingID == currentTarget.trackingID) {
            continue;
        }
        
        // Get color berdasarkan priority
        COLORREF lineColor = GetESPLineColor(monster, i);
        
        // Create pen untuk ESP line
        HPEN hPen = CreatePen(PS_SOLID, 1, lineColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        
        // Draw line dari player ke monster
        MoveToEx(hdc, offsetX + playerX, offsetY + playerY, NULL);
        LineTo(hdc, offsetX + monster.centerX, offsetY + monster.centerY);
        
        // Draw distance text
        char distText[32];
        sprintf_s(distText, sizeof(distText), "%.0f", monster.distToCenter);
        SetTextColor(hdc, lineColor);
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, offsetX + monster.centerX + 5, offsetY + monster.centerY - 15, distText, (int)strlen(distText));
        
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
    
    // Draw ESP line untuk current target dengan warna khusus
    if (isAttacking && currentTarget.pixelCount > 0) {
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));  // Cyan untuk current target
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        
        MoveToEx(hdc, offsetX + playerX, offsetY + playerY, NULL);
        LineTo(hdc, offsetX + currentTarget.centerX, offsetY + currentTarget.centerY);
        
        // Draw arrow atau indicator di tengah line
        int midX = (playerX + currentTarget.centerX) / 2;
        int midY = (playerY + currentTarget.centerY) / 2;
        
        // Draw small circle di tengah sebagai indicator
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 255, 255));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        Ellipse(hdc, offsetX + midX - 3, offsetY + midY - 3, 
                offsetX + midX + 3, offsetY + midY + 3);
        
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

void MonsterFinderBot::DrawOverlay() {
    if (!config.showBoxes && !config.showESPLines) return;
    
    HDC hdc = GetDC(gameWindow);
    if (!hdc) return;
    
    RECT rect;
    GetClientRect(gameWindow, &rect);
    int offsetX = (rect.right - rect.left) / 2 - config.scanWidth / 2;
    int offsetY = (rect.bottom - rect.top) / 2 - config.scanHeight / 2;
    
    // Draw ESP lines FIRST (di belakang boxes)
    if (config.showESPLines) {
        DrawESPLines(hdc, offsetX, offsetY);
    }
    
    // Draw all tracked monsters (each monster has its own box)
    if (config.showBoxes) {
        for (const auto& pair : trackedMonsters) {
            const BoundingBox& box = pair.second;
            
            // Skip if this is current target (will draw separately)
            if (isAttacking && box.trackingID == currentTarget.trackingID) {
                continue;
            }
            
            // Draw box in red for detected monsters
            // Box will follow monster movement automatically through tracking
            DrawBoundingBox(hdc, box, RGB(255, 0, 0), offsetX, offsetY);
        }
        
        // Draw current target box in yellow (on top, thicker)
        if (isAttacking && currentTarget.pixelCount > 0) {
            // Draw thicker box for current target
            HPEN hPen = CreatePen(PS_SOLID, 3, RGB(255, 255, 0));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            HBRUSH hBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
            
            Rectangle(hdc, offsetX + currentTarget.minX, offsetY + currentTarget.minY, 
                      offsetX + currentTarget.maxX, offsetY + currentTarget.maxY);
            
            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);
            DeleteObject(hPen);
        }
    }
    
    ReleaseDC(gameWindow, hdc);
}

DWORD MonsterFinderBot::ApplyRandomDelay(DWORD baseDelay) {
    if (config.randomDelay) {
        return config.randomDelayMin + rand() % (config.randomDelayMax - config.randomDelayMin);
    }
    return baseDelay;
}

bool MonsterFinderBot::CheckHotkeys() {
    vector<HotkeyAction> actions = hotkeyManager.CheckHotkeys();
    
    for (auto action : actions) {
        HandleHotkey(action);
    }
    
    return !actions.empty();
}

void MonsterFinderBot::HandleHotkey(HotkeyAction action) {
    switch (action) {
        case HK_START_BOT:
            Start();
            break;
        case HK_STOP_BOT:
            Stop();
            break;
        case HK_PAUSE_BOT:
            if (isPaused) Resume();
            else Pause();
            break;
        case HK_TOGGLE_AUTO_LOOT:
            config.autoLoot = !config.autoLoot;
            logger.Info("Auto Loot: " + string(config.autoLoot ? "ON" : "OFF"));
            break;
        case HK_TOGGLE_SHOW_BOXES:
            config.showBoxes = !config.showBoxes;
            logger.Info("Show Boxes: " + string(config.showBoxes ? "ON" : "OFF"));
            break;
        case HK_ESCAPE:
            inputManager->KeyPress(config.escapeKey);
            logger.Warning("Emergency escape triggered!");
            break;
        case HK_RELOAD_CONFIG:
            LoadAllConfigurations();
            logger.Info("Configuration reloaded");
            break;
        case HK_QUICK_TARGET:
            // Quick target nearest
            break;
        default:
            break;
    }
}

Statistics MonsterFinderBot::GetStats() const {
    return statsManager.GetStats();
}

string MonsterFinderBot::GetStatsString() const {
    return statsManager.GetFormattedStats();
}

void MonsterFinderBot::LogActivity(const string& message, LogLevel level) {
    logger.Log(level, message);
}
