#pragma once
// =========================================================
// CharacterData.h — Character roster with names, stats, descriptions
// No STL libraries
// =========================================================

#include <SFML/Graphics.hpp>

struct CharacterData
{
    const char* name;
    const char* description;
    const char* statLine;      // e.g. "SPD:★★★  PWR:★★  DEF:★"
    sf::Color   bodyColor;
    sf::Color   accentColor;
    float       speedMult;     // multiplier applied to PLAYER_SPEED
    float       throwRate;     // throw cooldown multiplier (lower = faster)
    int         startLives;    // override PLAYER_START_LIVES
    const char* imagePath;     // relative path to character portrait, e.g. "assets/images/BLOSSOM.png"
};

// =========================================================
// Global character roster — 6 selectable characters
// =========================================================
static const int CHARACTER_COUNT = 6;

static const CharacterData CHARACTER_ROSTER[CHARACTER_COUNT] =
{
    {
        "BLOSSOM",
        "A daring snowball fighter from the frozen north.\nBalanced stats — great for beginners.",
        "SPD: **  PWR: **  DEF: **",
        sf::Color(50, 100, 255),
        sf::Color(120, 160, 255),
        1.0f, 1.0f, 2,
        "assets/images/b0.png"
    },
    {
        "BUBBLE",
        "Nick's hot-headed rival. Throws faster but\nruns a little slower.",
        "SPD: *   PWR: *** DEF: **",
        sf::Color(255, 100, 50),
        sf::Color(255, 170, 120),
        0.85f, 0.70f, 2,
        "assets/images/b1.png"
    },
    {
        "BUTTERCUP",
        "Lightning-fast sprinter. Fragile but nearly\nimpossible to catch.",
        "SPD: *** PWR: *   DEF: *",
        sf::Color(80, 200, 100),
        sf::Color(160, 240, 170),
        1.45f, 1.10f, 1,
        "assets/images/b2.png"
    },
    {
        "RETIRED PROF.",
        "Tough brawler who takes an extra hit before\ngoing down. Slow but resilient.",
        "SPD: *   PWR: **  DEF: ***",
        sf::Color(100, 60, 180),
        sf::Color(170, 130, 255),
        0.75f, 1.0f, 3,
        "assets/images/b3.png"
    },
    {
        "DOCTOR",
        "Sharpshooter with extended natural throw range.\nAverage speed and defence.",
        "SPD: **  PWR: *** DEF: **",
        sf::Color(220, 60, 130),
        sf::Color(255, 140, 190),
        1.0f, 1.0f, 2,
        "assets/images/b4.png"
    },
    {
        "FIREBOY",
        "Mountain warrior. Extra life from the start\nand steady in every stat.",
        "SPD: **  PWR: **  DEF: ***",
        sf::Color(200, 140, 40),
        sf::Color(240, 200, 120),
        0.95f, 0.95f, 3,
        "assets/images/b5.png"
    }
};