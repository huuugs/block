#include "userManager.h"
#include <cstdio>
#include <cstring>

namespace BlockEater {

UserManager::UserManager()
    : currentUserIndex(-1)
{
}

UserManager::~UserManager() {
    shutdown();
}

void UserManager::init() {
    // Reset all users
    for (int i = 0; i < MAX_USERS; i++) {
        users[i].reset();
    }
    currentUserIndex = -1;

    // Try to load saved data
    if (loadFromFile()) {
        TraceLog(LOG_INFO, "User data loaded successfully");
    } else {
        TraceLog(LOG_INFO, "No existing user data found, starting fresh");
    }
}

void UserManager::shutdown() {
    // Save data before shutting down
    saveToFile();
}

int UserManager::createUser(const char* username) {
    if (!isValidUsername(username)) {
        TraceLog(LOG_ERROR, "Invalid username");
        return -1;
    }

    // Check if username already exists
    if (usernameExists(users, MAX_USERS, username)) {
        TraceLog(LOG_WARNING, "Username already exists");
        return -1;
    }

    // Find empty slot
    int slot = -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].isValid) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        TraceLog(LOG_ERROR, "No available user slots");
        return -1;
    }

    // Create new user
    users[slot].reset();
    strncpy(users[slot].username, username, MAX_USERNAME_LEN);
    users[slot].username[MAX_USERNAME_LEN] = '\0';
    users[slot].isValid = true;
    users[slot].lastPlayTime = (float)GetTime();

    currentUserIndex = slot;

    TraceLog(LOG_INFO, TextFormat("User created: %s at slot %d", username, slot));
    saveToFile();

    return slot;
}

bool UserManager::deleteUser(int userIndex) {
    if (userIndex < 0 || userIndex >= MAX_USERS) {
        return false;
    }

    if (!users[userIndex].isValid) {
        return false;
    }

    users[userIndex].reset();

    // If we deleted current user, reset selection
    if (currentUserIndex == userIndex) {
        currentUserIndex = -1;
    }

    saveToFile();
    return true;
}

int UserManager::findUser(const char* username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].isValid && strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int UserManager::getUserCount() const {
    int count = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].isValid) {
            count++;
        }
    }
    return count;
}

bool UserManager::saveToFile() {
    FILE* file = fopen(SAVE_FILE_PATH, "wb");
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open save file for writing");
        return false;
    }

    // Write magic number for validation
    const int MAGIC = 0x42455355;  // "BESU" - Block Eater Save User
    fwrite(&MAGIC, sizeof(int), 1, file);

    // Write version
    const int VERSION = 1;
    fwrite(&VERSION, sizeof(int), 1, file);

    // Write current user index
    fwrite(&currentUserIndex, sizeof(int), 1, file);

    // Write all users
    for (int i = 0; i < MAX_USERS; i++) {
        fwrite(&users[i], sizeof(User), 1, file);
    }

    fclose(file);
    TraceLog(LOG_INFO, "User data saved successfully");
    return true;
}

bool UserManager::loadFromFile() {
    FILE* file = fopen(SAVE_FILE_PATH, "rb");
    if (!file) {
        return false;
    }

    // Read and verify magic number
    int magic;
    if (fread(&magic, sizeof(int), 1, file) != 1 || magic != 0x42455355) {
        fclose(file);
        TraceLog(LOG_ERROR, "Invalid save file");
        return false;
    }

    // Read version
    int version;
    if (fread(&version, sizeof(int), 1, file) != 1) {
        fclose(file);
        return false;
    }

    // Read current user index
    if (fread(&currentUserIndex, sizeof(int), 1, file) != 1) {
        fclose(file);
        return false;
    }

    // Read all users
    for (int i = 0; i < MAX_USERS; i++) {
        if (fread(&users[i], sizeof(User), 1, file) != 1) {
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;
}

void UserManager::exportToFile(const char* path) {
    FILE* file = fopen(path, "w");
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open export file");
        return;
    }

    fprintf(file, "=== Block Eater User Data Export ===\n\n");

    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].isValid) {
            fprintf(file, "User %d: %s\n", i + 1, users[i].username);
            fprintf(file, "  Total Games: %d\n", users[i].totalGamesPlayed);
            fprintf(file, "  Total Score: %d\n", users[i].totalScore);
            fprintf(file, "  Play Time: %.0f seconds (%.1f hours)\n",
                    users[i].totalPlayTime, users[i].totalPlayTime / 3600.0f);
            fprintf(file, "  Endless High Score: %d\n", users[i].endlessStats.highScore);
            fprintf(file, "  Level Mode Best: %d\n", users[i].levelStats.highScore);
            fprintf(file, "  Time Challenge Best: %d\n", users[i].timeChallengeStats.highScore);
            fprintf(file, "\n");
        }
    }

    fclose(file);
    TraceLog(LOG_INFO, "User data exported successfully");
}

bool UserManager::importFromFile(const char* path) {
    // Simple text import - just for username list
    FILE* file = fopen(path, "r");
    if (!file) {
        return false;
    }

    char line[256];
    int imported = 0;

    while (fgets(line, sizeof(line), file) && imported < MAX_USERS) {
        // Skip empty lines and comments
        if (line[0] == '\n' || line[0] == '#') {
            continue;
        }

        // Remove newline
        line[strcspn(line, "\n")] = '\0';

        // Trim whitespace
        char* start = line;
        while (*start == ' ' || *start == '\t') start++;

        if (strlen(start) > 0 && createUser(start) >= 0) {
            imported++;
        }
    }

    fclose(file);
    TraceLog(LOG_INFO, TextFormat("Imported %d users", imported));
    return imported > 0;
}

void UserManager::updateStats(GameMode mode, int score, float playTime, int levelReached) {
    User* user = getCurrentUser();
    if (!user) {
        return;
    }

    user->lastPlayTime = (float)GetTime();
    user->totalPlayTime += playTime;
    user->totalGamesPlayed++;

    // Update mode-specific stats
    ModeStats* stats = nullptr;
    switch (mode) {
        case GameMode::ENDLESS:
            stats = &user->endlessStats;
            break;
        case GameMode::LEVEL:
            stats = &user->levelStats;
            if (levelReached > user->maxLevelUnlocked) {
                user->maxLevelUnlocked = levelReached;
            }
            break;
        case GameMode::TIME_CHALLENGE:
            stats = &user->timeChallengeStats;
            break;
    }

    if (stats) {
        stats->gamesPlayed++;
        stats->totalTimePlayed += playTime;
        if (score > stats->highScore) {
            stats->highScore = score;
        }
        if (levelReached > stats->highestLevel) {
            stats->highestLevel = levelReached;
        }
    }

    if (score > user->totalScore) {
        user->totalScore = score;
    }

    saveToFile();
}

void UserManager::updateHighScore(GameMode mode, int score) {
    User* user = getCurrentUser();
    if (!user) {
        return;
    }

    ModeStats* stats = nullptr;
    switch (mode) {
        case GameMode::ENDLESS:
            stats = &user->endlessStats;
            break;
        case GameMode::LEVEL:
            stats = &user->levelStats;
            break;
        case GameMode::TIME_CHALLENGE:
            stats = &user->timeChallengeStats;
            break;
    }

    if (stats && score > stats->highScore) {
        stats->highScore = score;
        saveToFile();
    }
}

void UserManager::incrementGamesPlayed(GameMode mode) {
    User* user = getCurrentUser();
    if (!user) {
        return;
    }

    user->totalGamesPlayed++;

    switch (mode) {
        case GameMode::ENDLESS:
            user->endlessStats.gamesPlayed++;
            break;
        case GameMode::LEVEL:
            user->levelStats.gamesPlayed++;
            break;
        case GameMode::TIME_CHALLENGE:
            user->timeChallengeStats.gamesPlayed++;
            break;
    }

    saveToFile();
}

void UserManager::addPlayTime(float time) {
    User* user = getCurrentUser();
    if (!user) {
        return;
    }

    user->totalPlayTime += time;
    user->lastPlayTime = (float)GetTime();
    saveToFile();
}

bool UserManager::isValidUsername(const char* username) {
    if (!username || strlen(username) == 0) {
        return false;
    }

    if (strlen(username) > MAX_USERNAME_LEN) {
        return false;
    }

    // Check for valid characters (Chinese, English, numbers)
    for (size_t i = 0; i < strlen(username); i++) {
        unsigned char c = username[i];
        // ASCII (English letters, numbers, basic symbols)
        if (c < 0x80) {
            if (!((c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') ||
                  c == '_' || c == '-')) {
                return false;
            }
        }
        // Chinese characters (UTF-8 multibyte)
        // Accept all UTF-8 sequences for Chinese
    }

    return true;
}

bool UserManager::usernameExists(const User* userList, int count, const char* username) {
    for (int i = 0; i < count; i++) {
        if (userList[i].isValid && strcmp(userList[i].username, username) == 0) {
            return true;
        }
    }
    return false;
}

void UserManager::sortUsersByLastPlay() {
    // Simple bubble sort by lastPlayTime
    for (int i = 0; i < MAX_USERS - 1; i++) {
        for (int j = 0; j < MAX_USERS - i - 1; j++) {
            if (users[j].isValid && users[j + 1].isValid &&
                users[j].lastPlayTime < users[j + 1].lastPlayTime) {
                User temp = users[j];
                users[j] = users[j + 1];
                users[j + 1] = temp;
            }
        }
    }
}

} // namespace BlockEater
