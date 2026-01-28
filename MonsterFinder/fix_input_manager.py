#!/usr/bin/env python3
import re

# Read the file
with open('MonsterFinderEnhanced.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# Replace inputManager. with inputManager->
content = re.sub(r'\binputManager\.', 'inputManager->', content)

# Write back
with open('MonsterFinderEnhanced.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("Replaced all inputManager. with inputManager->")
