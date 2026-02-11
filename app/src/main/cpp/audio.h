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
    static Sound GenerateShootSound();
    static Sound GenerateBlinkSound();
    static Sound GenerateShieldSound();
    static Sound GenerateRotateSound();
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
    void playShootSound();
    void playBlinkSound();
    void playShieldSound();
    void playRotateSound();
    void playBackgroundMusic(bool play = true);
    void updateMusic();

    void setMasterVolume(float volume);
    void setSfxVolume(float volume);
    void setMusicVolume(float volume);

    // Getters for volume
    float getMasterVolume() const { return masterVolume; }
    float getSfxVolume() const { return sfxVolume; }
    float getMusicVolume() const { return musicVolume; }
    bool isMuted() const { return m_isMuted; }

    // Mute/unmute
    void toggleMute() { m_isMuted = !m_isMuted; updateVolume(); }
    void setMuted(bool muted) { m_isMuted = muted; updateVolume(); }

private:
    Sound eatSounds[6];
    Sound hitSound;
    Sound levelUpSound;
    Sound deathSound;
    Sound buttonClickSound;
    Sound shootSound;
    Sound blinkSound;
    Sound shieldSound;
    Sound rotateSound;
    Music bgMusic;
    bool musicLoaded;

    float masterVolume;
    float sfxVolume;
    float musicVolume;
    bool musicPlaying;
    bool m_isMuted = false;
    float musicTime;
    float musicDuration;

    // Helper to update actual volume based on mute state
    void updateVolume() {
        float effectiveVolume = m_isMuted ? 0.0f : masterVolume;
        SetMasterVolume(effectiveVolume);
    }
};

} // namespace BlockEater

#endif // AUDIO_H
