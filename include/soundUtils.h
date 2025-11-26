/*
   SoundUtils.h - Utility classes for handling sound data and mixing multiple sound sources.
   Created by Chris, Sept. 2025.
   Released into the public domain.
*/

#ifndef SOUNDUTILS_H
#define SOUNDUTILS_H


class SoundData {
public:
    const int16_t* data;
    size_t len;
    float pos;
    float volume;
    float playbackSpeedFactor;
    bool loop = false;
    bool playing = false;

    // Constructor
    SoundData(const int16_t* dataSource, size_t dataLen, float defaultSpeed = 1.0f, float defaultPos = 0.0f, float defaultVolume = 1.0f)
        : data(dataSource), len(dataLen), pos(defaultPos), playbackSpeedFactor(defaultSpeed), volume(defaultVolume) {}

    // Set playback speed
    void setPlaybackSpeed(float speed) {
        playbackSpeedFactor = speed;
    }

    // Get current playback speed
    float getPlaybackSpeed() const {
        return playbackSpeedFactor;
    }

    // Set position
    void setPosition(float position) {
        pos = position;
    }

    // Set volume
    void setVolume(float vol) {
        volume = vol;
    }

    // Enable or disable looping
    void setLoop(bool shouldLoop) {
        loop = shouldLoop;
    }

    // Start playback
    void play() {
        pos = 0.0f;
        playing = true;
    
    }

    // Stop playback
    void stop() { 
        playing = false;
    }

    // Check if currently playing
    bool isPlaying() const { return playing; }
};


class SoundMixer {
public:
    std::vector<SoundData*> sources;

    // Constructor accepting multiple SoundData sources
    SoundMixer(std::initializer_list<SoundData*> srcs) {
        for (auto* src : srcs) {
            sources.push_back(src);
        }
    }

    // Default constructor
    SoundMixer() {}

    // Add a SoundData source
    void addSource(SoundData* source) {
        sources.push_back(source);
    }

    // Returns the sum of current samples from all sources
    int16_t getMixedSample() {
        int32_t sum = 0;
        for (auto* src : sources) {
            if (src && src->data && src->playing && (int)src->pos < src->len) {
                sum += src->data[(int)src->pos];
            }
        }
        // Clamp to int16_t range
        if (sum > INT16_MAX) sum = INT16_MAX;
        if (sum < INT16_MIN) sum = INT16_MIN;
        return (int16_t)sum;
    }

    // Progress the positions of all SoundData sources by their playbackSpeedFactor
    void advancePositions() {
        for (auto* src : sources) {
            if (src) {
                src->pos += src->playbackSpeedFactor;
                if ((int)src->pos >= src->len) {
                    if (!src->loop) {
                        src->playing = false; // Stop playback if not looping
                        src->pos = src->len; // Clamp to end
                    } else {
                        src->pos = 0.0f; // Loop back to start
                    }
                }
            }
        }
    }
};


#endif // SOUNDUTILS_H