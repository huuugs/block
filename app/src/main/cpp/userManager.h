#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "user.h"
#include "game.h"

namespace BlockEater {

class UserManager {
public:
    static constexpr int MAX_USERS = 5;        // Maximum user slots
    static constexpr int MAX_USERNAME_LEN = 63;
    static constexpr char SAVE_FILE_PATH[] = "user_data.dat";

    UserManager();
    ~UserManager();

    void init();
    void shutdown();

    // User management
    int createUser(const char* username);
    bool deleteUser(int userIndex);
    int findUser(const char* username);
    int getCurrentUserIndex() const { return currentUserIndex; }
    void setCurrentUser(int index) { currentUserIndex = index; }
    User* getCurrentUser() {
        return (currentUserIndex >= 0 && currentUserIndex < MAX_USERS) ?
               &users[currentUserIndex] : nullptr;
    }
    const User* getCurrentUser() const {
        return (currentUserIndex >= 0 && currentUserIndex < MAX_USERS) ?
               &users[currentUserIndex] : nullptr;
    }

    // User list
    int getUserCount() const;
    User* getUser(int index) {
        return (index >= 0 && index < MAX_USERS) ? &users[index] : nullptr;
    }
    const User* getUser(int index) const {
        return (index >= 0 && index < MAX_USERS) ? &users[index] : nullptr;
    }

    // Save/Load
    bool saveToFile();
    bool loadFromFile();
    void exportToFile(const char* path);
    bool importFromFile(const char* path);

    // Statistics update (called during gameplay)
    void updateStats(GameMode mode, int score, float playTime, int levelReached);
    void updateHighScore(GameMode mode, int score);
    void incrementGamesPlayed(GameMode mode);
    void addPlayTime(float time);

    // Validation
    static bool isValidUsername(const char* username);
    static bool usernameExists(const User* userList, int count, const char* username);

private:
    User users[MAX_USERS];
    int currentUserIndex;  // -1 = no user selected

    void sortUsersByLastPlay();
};

} // namespace BlockEater

#endif // USERMANAGER_H
