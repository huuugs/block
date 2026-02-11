#include "audio.h"
#include <cstring>
#include <cmath>

namespace BlockEater {

// Audio Generator Implementation
void AudioGenerator::GenerateWave(short* buffer, int size, int frequency, float duration,
                                 WaveType type, float volume) {
    int sampleRate = 44100;
    int samples = (int)(sampleRate * duration);

    for (int i = 0; i < size && i < samples; i++) {
        float t = (float)i / sampleRate;
        float sample = 0.0f;

        switch (type) {
            case WAVE_SINE:
                sample = sinf(2.0f * PI * frequency * t);
                break;
            case WAVE_SQUARE:
                sample = (sinf(2.0f * PI * frequency * t) > 0) ? 1.0f : -1.0f;
                break;
            case WAVE_SAWTOOTH:
                sample = 2.0f * (t * frequency - floorf(0.5f + t * frequency));
                break;
            case WAVE_TRIANGLE:
                sample = fabsf(2.0f * (t * frequency - floorf(0.5f + t * frequency))) - 1.0f;
                break;
        }

        // Apply volume
        sample *= volume * 0.3f;

        // Convert to 16-bit
        buffer[i] = (short)(sample * 32767.0f);
    }
}

void AudioGenerator::ApplyEnvelope(short* buffer, int size, float attack, float decay,
                                  float sustain, float release) {
    int sampleRate = 44100;
    int attackSamples = (int)(attack * sampleRate);
    int decaySamples = (int)(decay * sampleRate);
    int releaseSamples = (int)(release * sampleRate);

    for (int i = 0; i < size; i++) {
        float multiplier = 1.0f;

        if (i < attackSamples) {
            multiplier = (float)i / attackSamples;
        } else if (i < attackSamples + decaySamples) {
            float decayProgress = (float)(i - attackSamples) / decaySamples;
            multiplier = 1.0f - (1.0f - sustain) * decayProgress;
        } else if (i > size - releaseSamples) {
            int releaseIndex = i - (size - releaseSamples);
            multiplier = sustain * (1.0f - (float)releaseIndex / releaseSamples);
        }

        buffer[i] = (short)(buffer[i] * multiplier);
    }
}

Sound AudioGenerator::GenerateEatSound(int level) {
    int sampleRate = 44100;
    float duration = 0.15f;
    int samples = (int)(sampleRate * duration);
    short* buffer = new short[samples];

    // Higher pitch for higher levels
    int frequency = 200 + level * 100;

    GenerateWave(buffer, samples, frequency, duration, WAVE_SQUARE, 0.5f);
    ApplyEnvelope(buffer, samples, 0.01f, 0.05f, 0.3f, 0.05f);

    Wave wave = {
        .frameCount = static_cast<unsigned int>(samples),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    return LoadSoundFromWave(wave);
}

Sound AudioGenerator::GenerateHitSound() {
    int sampleRate = 44100;
    float duration = 0.2f;
    int samples = (int)(sampleRate * duration);
    short* buffer = new short[samples];

    GenerateWave(buffer, samples, 100, duration, WAVE_SAWTOOTH, 0.6f);
    ApplyEnvelope(buffer, samples, 0.01f, 0.1f, 0.0f, 0.1f);

    Wave wave = {
        .frameCount = static_cast<unsigned int>(samples),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    return LoadSoundFromWave(wave);
}

Sound AudioGenerator::GenerateLevelUpSound() {
    int sampleRate = 44100;
    float duration = 0.6f;
    int samples = (int)(sampleRate * duration);
    short* buffer = new short[samples];

    // Ascending arpeggio
    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        float noteDuration = duration / 4.0f;
        int note = (int)(t / noteDuration);

        int frequencies[] = {523, 659, 784, 1047};  // C5, E5, G5, C6
        int freq = frequencies[note < 4 ? note : 3];

        float sample = sinf(2.0f * PI * freq * t) * 0.3f;
        buffer[i] = (short)(sample * 32767.0f);
    }

    ApplyEnvelope(buffer, samples, 0.05f, 0.2f, 0.0f, 0.2f);

    Wave wave = {
        .frameCount = static_cast<unsigned int>(samples),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    return LoadSoundFromWave(wave);
}

Sound AudioGenerator::GenerateDeathSound() {
    int sampleRate = 44100;
    float duration = 0.5f;
    int samples = (int)(sampleRate * duration);
    short* buffer = new short[samples];

    // Descending tone
    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        int freq = 400 - (int)(t * 600);
        if (freq < 50) freq = 50;

        float sample = sinf(2.0f * PI * freq * t) * 0.4f;
        buffer[i] = (short)(sample * 32767.0f);
    }

    ApplyEnvelope(buffer, samples, 0.01f, 0.0f, 0.0f, 0.4f);

    Wave wave = {
        .frameCount = static_cast<unsigned int>(samples),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    return LoadSoundFromWave(wave);
}

Sound AudioGenerator::GenerateButtonClickSound() {
    int sampleRate = 44100;
    float duration = 0.1f;
    int samples = (int)(sampleRate * duration);
    short* buffer = new short[samples];

    GenerateWave(buffer, samples, 800, duration, WAVE_SINE, 0.3f);
    ApplyEnvelope(buffer, samples, 0.01f, 0.02f, 0.0f, 0.02f);

    Wave wave = {
        .frameCount = static_cast<unsigned int>(samples),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    return LoadSoundFromWave(wave);
}

Music AudioGenerator::GenerateBackgroundMusic() {
    // For simplicity, return empty music
    // In a full implementation, you would generate a chiptune loop
    Music music = {0};
    return music;
}

// Audio Manager Implementation
AudioManager::AudioManager()
    : masterVolume(1.0f)
    , sfxVolume(0.8f)
    , musicVolume(0.6f)
    , musicPlaying(false)
{
}

AudioManager::~AudioManager() {
}

void AudioManager::init() {
    // Generate all sounds
    for (int i = 0; i < 6; i++) {
        eatSounds[i] = AudioGenerator::GenerateEatSound(i + 1);
    }

    hitSound = AudioGenerator::GenerateHitSound();
    levelUpSound = AudioGenerator::GenerateLevelUpSound();
    deathSound = AudioGenerator::GenerateDeathSound();
    buttonClickSound = AudioGenerator::GenerateButtonClickSound();
}

void AudioManager::shutdown() {
    for (int i = 0; i < 6; i++) {
        UnloadSound(eatSounds[i]);
    }
    UnloadSound(hitSound);
    UnloadSound(levelUpSound);
    UnloadSound(deathSound);
    UnloadSound(buttonClickSound);
}

void AudioManager::playEatSound(int level) {
    int index = (level - 1) % 6;
    PlaySound(eatSounds[index]);
}

void AudioManager::playHitSound() {
    PlaySound(hitSound);
}

void AudioManager::playLevelUpSound() {
    PlaySound(levelUpSound);
}

void AudioManager::playDeathSound() {
    PlaySound(deathSound);
}

void AudioManager::playButtonClickSound() {
    PlaySound(buttonClickSound);
}

void AudioManager::playBackgroundMusic(bool play) {
    musicPlaying = play;
}

void AudioManager::setMasterVolume(float volume) {
    masterVolume = fmaxf(0.0f, fminf(1.0f, volume));
    updateVolume();
}

void AudioManager::setSfxVolume(float volume) {
    sfxVolume = fmaxf(0.0f, fminf(1.0f, volume));
}

void AudioManager::setMusicVolume(float volume) {
    musicVolume = fmaxf(0.0f, fminf(1.0f, volume));
}

} // namespace BlockEater
