#!/usr/bin/env python3

# Read file
with open('MonsterFinderEnhanced.h', 'r', encoding='utf-8') as f:
    content = f.read()

# Add GetStats() method back to StatsManager
old_section = """    void RecordPotionUsed();
    void UpdateStats();
    string GetFormattedStats() const;
    void SaveToFile(const string& filename);"""

new_section = """    void RecordPotionUsed();
    void UpdateStats();
    Statistics GetStats() const { return stats; }
    string GetFormattedStats() const;
    void SaveToFile(const string& filename);"""

content = content.replace(old_section, new_section)

# Write back
with open('MonsterFinderEnhanced.h', 'w', encoding='utf-8') as f:
    f.write(content)

print("Added GetStats() inline function to StatsManager")
