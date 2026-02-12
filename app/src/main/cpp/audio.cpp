#include "audio.h"
#include <cstring>
#include <cmath>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

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
    // Generate space-themed 8-bit background music
    int sampleRate = 44100;
    float duration = 32.0f;  // 32 seconds looping
    int samples = (int)(sampleRate * duration);

    // Calculate data size (16-bit = 2 bytes per sample, mono = 1 channel)
    int dataSize = samples * 2;
    unsigned char* dataBuffer = new unsigned char[dataSize];

    // Space music notes - ambient, atmospheric
    // Using a pentatonic scale for spacey feel: C4, D4, E4, G4, A4
    int baseNotes[] = {262, 294, 330, 392, 440};  // C4, D4, E4, G4, A4
    int bassNotes[] = {131, 147, 165, 196, 220};   // C3, D3, E3, G3, A3 (octave down)

    // Music structure: 8 measures, 4 beats each
    const float BEAT_DURATION = 0.5f;  // 120 BPM
    const int BEATS_PER_PHRASE = 16;

    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        float sample = 0.0f;

        // Determine current beat in phrase
        int beatInPhrase = (int)(t / BEAT_DURATION) % BEATS_PER_PHRASE;

        // Melody: Simple spacey arpeggio pattern
        int melodyPattern[] = {0, 2, 4, 2, 0, 4, 2, 0, 3, 4, 2, 1, 0, 1, 2, 4};
        int noteIndex = melodyPattern[beatInPhrase];
        int melodyFreq = baseNotes[noteIndex % 5];

        // Melody volume envelope - pulsing
        float melodyEnvelope = 0.15f + 0.05f * sinf(t * 2.0f);

        // Square wave for melody (8-bit sound)
        float vibrato = 1.0f + 0.02f * sinf(t * 8.0f);
        float melodyWave = (sinf(2.0f * PI * melodyFreq * vibrato * t) > 0) ? 1.0f : -1.0f;

        // Bass pattern: slower, deeper notes
        int bassPattern[] = {0, 0, 4, 4, 0, 3, 2, 2, 0, 0, 4, 4, 2, 1, 0, 0};
        int bassNoteIndex = bassPattern[beatInPhrase];
        int bassFreq = bassNotes[bassNoteIndex % 5];

        // Triangle wave for bass (softer)
        float bassPhase = fmodf(t * bassFreq, 1.0f);
        float bassWave = fabsf(2.0f * bassPhase - 1.0f) * 2.0f - 1.0f;
        float bassEnvelope = 0.2f;

        // Pad/ambient layer - slow sine wave with harmonics
        float padFreq = 196.0f;  // G3
        float padWave = sinf(2.0f * PI * padFreq * t) * 0.08f +
                       sinf(2.0f * PI * padFreq * 1.5f * t) * 0.04f +
                       sinf(2.0f * PI * padFreq * 2.0f * t) * 0.02f;

        // Arpeggiator effect on beats
        float arpWave = 0;
        if (beatInPhrase % 2 == 0) {
            float arpFreq = 523.0f;  // C5
            float arpData = fmodf(t * 8.0f, 1.0f);
            arpWave = (sinf(2.0f * PI * arpFreq * t) > 0) ? 1.0f : -1.0f;
            arpWave *= 0.03f * (1.0f - arpData);  // Quick decay
        }

        // Mix all layers
        sample = melodyWave * melodyEnvelope +
                 bassWave * bassEnvelope +
                 padWave +
                 arpWave;

        // Apply overall song envelope (fade in/out at ends)
        float songEnvelope = 1.0f;
        if (t < 2.0f) {
            songEnvelope = t / 2.0f;  // Fade in
        } else if (t > duration - 2.0f) {
            songEnvelope = (duration - t) / 2.0f;  // Fade out
        }

        // Master volume reduction
        sample *= songEnvelope * 0.25f;

        // Convert to 16-bit PCM and store in little-endian format
        short sample16 = (short)(sample * 32767.0f);
        dataBuffer[i * 2] = sample16 & 0xFF;           // Low byte
        dataBuffer[i * 2 + 1] = (sample16 >> 8) & 0xFF;  // High byte
    }

    // Load music from memory buffer
    // LoadMusicStreamFromMemory requires: fileType, data, dataSize
    Music music = LoadMusicStreamFromMemory(".ogg", dataBuffer, dataSize);

    // Clean up the data buffer (raylib makes its own copy)
    delete[] dataBuffer;

    return music;
}

// Shoot sound - high pitch laser
Sound AudioGenerator::GenerateShootSound() {
    int sampleRate = 44100;
    float duration = 0.15f;
    int samples = (int)(sampleRate * duration);
    short* buffer = new short[samples];

    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        // Descending laser pitch
        int freq = 800 - (int)(t * 1200);
        if (freq < 100) freq = 100;

        float sample = sinf(2.0f * PI * freq * t) * 0.3f;
        buffer[i] = (short)(sample * 32767.0f);
    }

    ApplyEnvelope(buffer, samples, 0.01f, 0.05f, 0.0f, 0.05f);

    Wave wave = {
        .frameCount = static_cast<unsigned int>(samples),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    return LoadSoundFromWave(wave);
}

// Blink sound - teleport whoosh
Sound AudioGenerator::GenerateBlinkSound() {
    int sampleRate = 44100;
    float duration = 0.2f;
    int samples = (int)(sampleRate * duration);
    short* buffer = new short[samples];

    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        // Rising then falling pitch
        float phase = (t < duration / 2) ? (t / (duration / 2)) : (1.0f - (t - duration / 2) / (duration / 2));
        int freq = 200 + (int)(phase * 600);

        float sample = (sinf(2.0f * PI * freq * t) > 0 ? 1.0f : -1.0f) * 0.3f;
        buffer[i] = (short)(sample * 32767.0f);
    }

    ApplyEnvelope(buffer, samples, 0.02f, 0.08f, 0.0f, 0.08f);

    Wave wave = {
        .frameCount = static_cast<unsigned int>(samples),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    return LoadSoundFromWave(wave);
}

// Shield sound - power up hum
Sound AudioGenerator::GenerateShieldSound() {
    int sampleRate = 44100;
    float duration = 0.3f;
    int samples = (int)(sampleRate * duration);
    short* buffer = new short[samples];

    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        // Rising pitch with harmonics
        int freq = 150 + (int)(t * 400);

        float sample = sinf(2.0f * PI * freq * t) * 0.25f +
                      sinf(2.0f * PI * freq * 2 * t) * 0.15f;
        buffer[i] = (short)(sample * 32767.0f);
    }

    ApplyEnvelope(buffer, samples, 0.05f, 0.15f, 0.0f, 0.15f);

    Wave wave = {
        .frameCount = static_cast<unsigned int>(samples),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    return LoadSoundFromWave(wave);
}

// Rotate sound - spinning effect
Sound AudioGenerator::GenerateRotateSound() {
    int sampleRate = 44100;
    float duration = 0.25f;
    int samples = (int)(sampleRate * duration);
    short* buffer = new short[samples];

    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        // Oscillating frequency for spinning effect
        float wobble = sinf(t * 20.0f);
        int freq = 300 + (int)(wobble * 100);

        float sample = (sinf(2.0f * PI * freq * t) > 0 ? 1.0f : -1.0f) * 0.35f;
        buffer[i] = (short)(sample * 32767.0f);
    }

    ApplyEnvelope(buffer, samples, 0.02f, 0.1f, 0.0f, 0.1f);

    Wave wave = {
        .frameCount = static_cast<unsigned int>(samples),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    return LoadSoundFromWave(wave);
}

// Audio Manager Implementation
AudioManager::AudioManager()
    : masterVolume(1.0f)
    , sfxVolume(0.8f)
    , musicVolume(0.6f)
    , musicPlaying(false)
    , musicLoaded(false)
    , musicTime(0.0f)
    , musicDuration(16.0f)
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
    shootSound = AudioGenerator::GenerateShootSound();
    blinkSound = AudioGenerator::GenerateBlinkSound();
    shieldSound = AudioGenerator::GenerateShieldSound();
    rotateSound = AudioGenerator::GenerateRotateSound();

    // Generate and load background music
    // IMPORTANT: Don't check musicLoaded status - always try to play even if load might have failed
    // The music might not load properly on some devices, so we try anyway
    bgMusic = AudioGenerator::GenerateBackgroundMusic();
    TraceLog(LOG_INFO, "Background music generated, stream buffer: %p", bgMusic.stream.buffer);
}

void AudioManager::shutdown() {
    for (int i = 0; i < 6; i++) {
        UnloadSound(eatSounds[i]);
    }
    UnloadSound(hitSound);
    UnloadSound(levelUpSound);
    UnloadSound(deathSound);
    UnloadSound(buttonClickSound);
    UnloadSound(shootSound);
    UnloadSound(blinkSound);
    UnloadSound(shieldSound);
    UnloadSound(rotateSound);

    if (musicLoaded) {
        UnloadMusicStream(bgMusic);
        musicLoaded = false;
    }
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

void AudioManager::playShootSound() {
    PlaySound(shootSound);
}

void AudioManager::playBlinkSound() {
    PlaySound(blinkSound);
}

void AudioManager::playShieldSound() {
    PlaySound(shieldSound);
}

void AudioManager::playRotateSound() {
    PlaySound(rotateSound);
}

void AudioManager::playBackgroundMusic(bool play) {
    // IMPORTANT: Don't check musicLoaded - try to play anyway
    // This allows game to attempt playing even if music load failed
    if (play && !musicPlaying) {
        PlayMusicStream(bgMusic);
        musicPlaying = true;
        TraceLog(LOG_INFO, "Background music play requested");
    } else if (!play && musicPlaying) {
        StopMusicStream(bgMusic);
        musicPlaying = false;
        TraceLog(LOG_INFO, "Background music stop requested");
    }
}

void AudioManager::updateMusic() {
    if (musicLoaded && musicPlaying) {
        UpdateMusicStream(bgMusic);
    }
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
