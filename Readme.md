# To Sharpshot Or Not To Sharpshot
Welcome! This is my D&D optimizer I worked on for an afternoon. When it comes to dungeons and dragons, I love to take a character concept and make them as powerful as possible within the bounds of the game and the fantasy of the character (and the enjoyment of the other players). Often this involves the sharpshooter or great weapon master feats, or custom options that function similarly. I am never sure what the cutoff point of the best option to use is, so I made this app! It doubles as a great way to find your average damage on a particular AC, as it calculates crits.

## Function
This program takes user specifications and runs dice roll trials to achieve an average damage on a number of armor classes. It rolls 6 sets of dice, 2 with advantage, 2 with disadvantage, and one standard. Each of these sets has one rolled as a gamble and one rolled normally.

## Programming Specification
This program was created on a windows machine with visual studio code. It uses OpenGL, GLFW, glad, and DearImGui.

## User Specifications
This program allows for the following specifications:
- Crit Ranges from 18-20
- Brutal Critical
- Halfling Reroll
- Hunter's Mark/Hex/Once per turn damages
- Variable Trials
- Attacks with multiple damage types
- Attacks with multiple damage die sizes
- Attacks with multiple dice rolled
This program does NOT support:
- Spellcasters. Martials need something, someone already made a google doc of the spells... (I may revisit this later to add this).
- Advanced Homebrewing support (Unfortunately, I cannot plan for each 6 rolled to double damage or anything).
- Minor flat damage increases. (I'll probably revisit this, but things like nat 20 extra damage on the dragon's wrath weapons and flat damage even if you miss aren't currently implemented).

## Next Steps
- Resistance
- Guaranteed Criticals on Hits
- Flat Damage
- Reroll Threshhold

## How to download
Just download the zip file or clone the repo!