#!/usr/bin/env python3

# Read header file
with open('MonsterFinderEnhanced.h', 'r', encoding='utf-8') as f:
    header_content = f.read()

# Check if GetStats is declared in StatsManager
if 'Statistics GetStats() const;' in header_content:
    print("✓ GetStats() const is declared in StatsManager class")
else:
    print("✗ GetStats() const is NOT declared in StatsManager class")

# Read cpp file
with open('MonsterFinderEnhanced.cpp', 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# Check if GetStats is implemented in StatsManager
if 'Statistics StatsManager::GetStats() const' in cpp_content:
    print("✓ GetStats() const is implemented in MonsterFinderEnhanced.cpp")
else:
    print("✗ GetStats() const is NOT implemented in MonsterFinderEnhanced.cpp")

# Check if MonsterFinderBot::GetStats calls StatsManager::GetStats
if 'return statsManager.GetStats()' in cpp_content:
    print("✓ MonsterFinderBot::GetStats() calls statsManager.GetStats()")
else:
    print("✗ MonsterFinderBot::GetStats() does NOT call statsManager.GetStats()")
