#ifndef USER_H
#define USER_H

#include "raylib.h"
#include <cstring>

namespace BlockEater {

// User statistics for each game mode
struct ModeStats {
    int highScore;          // Highest score
    int gamesPlayed;        // Number of games played
    float totalTimePlayed;  // Total time in seconds
    int highestLevel;       // Highest level reached
};

// Full user profile
struct User {
    char username[64];      // User name (Chinese or English)
    bool isValid;           // Is this user slot valid/active?

    // Overall statistics
    int totalGamesPlayed;
    float totalPlayTime;    // In seconds
    int totalScore;

    // Mode-specific statistics
    ModeStats endlessStats;
    ModeStats levelStats;
    ModeStats timeChallengeStats;

    // Progress tracking
    int maxLevelUnlocked;   // Highest level available
    int achievements;       // Achievement count

    // Last played
    float lastPlayTime;     // Unix timestamp of last play

    // Constructor
    User() : isValid(false) {
        memset(username, 0, sizeof(username));
        totalGamesPlayed = 0;
        totalPlayTime = 0;
        totalScore = 0;
        maxLevelUnlocked = 1;
        achievements = 0;
        lastPlayTime = 0;

        endlessStats = {0, 0, 0, 1};
        levelStats = {0, 0, 0, 1};
        timeChallengeStats = {0, 0, 0, 0};
    }

    // Reset user data
    void reset() {
        memset(username, 0, sizeof(username));
        isValid = false;
        totalGamesPlayed = 0;
        totalPlayTime = 0;
        totalScore = 0;
        maxLevelUnlocked = 1;
        achievements = 0;
        lastPlayTime = 0;

        endlessStats = {0, 0, 0, 1};
        levelStats = {0, 0, 0, 1};
        timeChallengeStats = {0, 0, 0, 0};
    }
};

} // namespace BlockEater

#endif // USER_H
