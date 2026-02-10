#ifndef AUDIO_H
#define AUDIO_H

#include "raylib.h"
#include "game.h"

namespace BlockEater {

// Wave type enumeration for audio generation
enum WaveType {
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_SAWTOOTH,
    WAVE_TRIANGLE
};

// 8-bit sound generator using procedural audio
class AudioGenerator {
public:
    // Generate 8-bit style sounds
    static Sound GenerateEatSound(int level);
    static Sound GenerateHitSound();
    static Sound GenerateLevelUpSound();
    static Sound GenerateDeathSound();
    static Sound GenerateButtonClickSound();
    static Music GenerateBackgroundMusic();

private:
    static void GenerateWave(short* buffer, int size, int frequency, float duration,
                           WaveType type, float volume = 1.0f);
    static void ApplyEnvelope(short* buffer, int size, float attack, float decay, float sustain, float release);
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    void init();
    void shutdown();

    void playEatSound(int level);
    void playHitSound();
    void playLevelUpSound();
    void playDeathSound();
    void playButtonClickSound();
    void playBackgroundMusic(bool play = true);

    void setMasterVolume(float volume);
    void setSfxVolume(float volume);
    void setMusicVolume(float volume);

private:
    Sound eatSounds[6];
    Sound hitSound;
    Sound levelUpSound;
    Sound deathSound;
    Sound buttonClickSound;
    Music bgMusic;

    float masterVolume;
    float sfxVolume;
    float musicVolume;
    bool musicPlaying;
};

} // namespace BlockEater

#endif // AUDIO_H
