#pragma once

// =========================================================
// Constants.h
// Global game constants for Snow Bros - Bhool Bhulaiyaa
// =========================================================

// --- Window ---
const int   WINDOW_WIDTH = 800;
const int   WINDOW_HEIGHT = 600;
const int   TARGET_FPS = 60;
const char* const WINDOW_TITLE = "Snow Bros - Bhool Bhulaiyaa";

// --- Physics ---
// IMPORTANT: Only one GRAVITY definition. Removed the conflicting #define.
const float GRAVITY = 2000.f;   // px/s²  (was duplicated — fixed)
const float JUMP_FORCE = -400.f;   // enough to reach platforms ~120 px above
const float PLAYER_SPEED = 200.f;

// --- Snowball ---
const float SNOWBALL_SPEED = 400.f;
const float SNOWBALL_ROLL_SPEED = 300.f;
const int   HITS_TO_ENCASE_BOTOM = 2;

// --- Player ---
const int   PLAYER_START_LIVES = 2;
const float PLAYER_INVINCIBLE_TIME = 2.0f;   // seconds after being hit

// --- Power-up durations (seconds) ---
const float POWERUP_SPEED_DURATION = 15.0f;
const float POWERUP_BALLOON_DURATION = 10.0f;
const float POWERUP_SPEED_MULTIPLIER = 1.5f;

// --- Scoring ---
const int   SCORE_BOTOM_MIN = 100;
const int   SCORE_BOTOM_MAX = 500;
const int   SCORE_FOOGA_MIN = 200;
const int   SCORE_FOOGA_MAX = 800;
const int   SCORE_TORNADO_MIN = 300;
const int   SCORE_TORNADO_MAX = 1200;
const int   SCORE_MOGERA = 5000;
const int   SCORE_GAMAKICHI = 10000;

// Small enemy score (used by GamakichiChild and similar minions)
const int   SCORE_ENEMY_SMALL = 300;

const float CHAIN_BONUS_PERCENT = 0.10f;   // 10 % per chain kill

// --- Gems ---
const int GEMS_MOGERA_REWARD = 200;
const int GEMS_GAMAKICHI_REWARD = 500;

// --- Shop prices ---
const int SHOP_EXTRA_LIFE_COST = 50;
const int SHOP_SPEED_COST = 20;
const int SHOP_SNOWBALL_POWER_COST = 30;
const int SHOP_DISTANCE_COST = 25;
const int SHOP_BALLOON_COST = 35;

// --- Game ---
const int TOTAL_LEVELS = 10;
const int BOSS_LEVEL_1 = 5;
const int BOSS_LEVEL_2 = 10;
const int BONUS_LEVEL_COUNT = 2;