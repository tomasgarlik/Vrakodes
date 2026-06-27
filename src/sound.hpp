#include "sdl_includes.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

// Audio state
struct AudioData {
    std::vector<int16_t> samples;  // Raw PCM samples
    size_t position;               // Current playback position
    float speed;                   // Playback speed multiplier
    float sub_position;            // Fractional position for resampling
    bool looping;
    SDL_AudioDeviceID device;
};

static AudioData g_audio;
static std::vector<int16_t> g_sound1;
static std::vector<int16_t> g_sound2;
static const char* g_car_name = nullptr;

// Audio callback - SDL calls this to fill the audio buffer
void audio_callback(void* userdata, Uint8* stream, int len) {
    AudioData* audio = (AudioData*)userdata;
    
    int16_t* output = (int16_t*)stream;
    int samples_needed = len / sizeof(int16_t);
    
    // Clear buffer first
    memset(stream, 0, len);
    
    if (audio->samples.empty()) return;
    
    // Loop a bit before the actual end to avoid clicks
    size_t loop_point = audio->samples.size() - 100;
    
    for (int i = 0; i < samples_needed; i++) {
        // Check bounds
        if (audio->position >= loop_point) {
            if (audio->looping) {
                audio->position = 0;
                audio->sub_position = 0;
            } else {
                break;
            }
        }
        
        // Get current position in samples array
        size_t idx = audio->position;
        
        if (idx < audio->samples.size()) {
            output[i] = audio->samples[idx];
        }
        
        // Advance position by speed multiplier
        audio->sub_position += audio->speed;
        audio->position = (size_t)audio->sub_position;
    }
}


bool audio_init(int sample_rate = 48000) {
    printf("initiliasing audio\n");
    SDL_AudioSpec want, have;
    SDL_zero(want);
    
    want.freq = sample_rate;
    want.format = AUDIO_S16SYS;  // 16-bit signed samples
    want.channels = 2;            // Stereo
    want.samples = 4096;          // Buffer size
    want.callback = audio_callback;
    want.userdata = &g_audio;
    
    g_audio.device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    
    if (g_audio.device == 0) {
        std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
        return false;
    }
    
    g_audio.position = 0;
    g_audio.sub_position = 0;
    g_audio.speed = 1.0f;
    g_audio.looping = true;
    
    printf("Audio initialized: %d Hz, %d channels\n", have.freq, have.channels);
    
    return true;
}

bool audio_load_wav(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    
    // Skip WAV header (44 bytes)
    file.seekg(44);
    
    // Read all PCM data
    g_audio.samples.clear();
    int16_t sample;
    while (file.read((char*)&sample, sizeof(sample))) {
        g_audio.samples.push_back(sample);
    }
    
    printf("Loaded %zu samples from %s\n", g_audio.samples.size(), filename);
    
    return true;
}

bool audio_load_sound1() {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "assets/cars/%s/engine1.wav", car_name.c_str());
    printf("Loading sound1: %s\n", buffer);
    
    std::ifstream file(buffer, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << buffer << std::endl;
        return false;
    }
    
    file.seekg(44);
    g_sound1.clear();
    int16_t sample;
    while (file.read((char*)&sample, sizeof(sample))) {
        g_sound1.push_back(sample);
    }
    
    printf("Loaded %zu samples\n", g_sound1.size());
    return true;
}

bool audio_load_sound2() {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "assets/cars/%s/engine2.wav", car_name.c_str());
    printf("Loading sound2: %s\n", buffer);
    
    std::ifstream file(buffer, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << buffer << std::endl;
        return false;
    }
    
    file.seekg(44);
    g_sound2.clear();
    int16_t sample;
    while (file.read((char*)&sample, sizeof(sample))) {
        g_sound2.push_back(sample);
    }
    
    printf("Loaded %zu samples\n", g_sound2.size());
    return true;
}

void audio_switch_to_sound1() {
    g_audio.samples = g_sound1;
    g_audio.position = 0;
    g_audio.sub_position = 0;
}

void audio_switch_to_sound2() {
    g_audio.samples = g_sound2;
    g_audio.position = 0;
    g_audio.sub_position = 0;
}

void audio_start_loop() {
    printf("[audio] starting audio loop\n");
    g_audio.position = 0;
    g_audio.sub_position = 0;
    SDL_PauseAudioDevice(g_audio.device, 0);
}

void audio_play() {
    g_audio.position = 0;
    g_audio.sub_position = 0;
    SDL_PauseAudioDevice(g_audio.device, 0);  // Start playing
}

void audio_stop() {
    SDL_PauseAudioDevice(g_audio.device, 1);  // Pause
}

void audio_set_speed(float speed) {
    // Clamp to reasonable range
    if (speed < 0.5f) speed = 0.5f;
    if (speed > 2.0f) speed = 2.0f;
    g_audio.speed = speed;
}

void audio_set_looping(bool loop) {
    g_audio.looping = loop;
}

void audio_update() {
    // This function exists for compatibility but doesn't need to do anything
    // Audio playback happens automatically in the callback
}

void audio_cleanup() {
    SDL_CloseAudioDevice(g_audio.device);
}

// Example usage
int mainer_schwester(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_AUDIO);
    
    // Init audio
    if (!audio_init(48000)) {
        return -1;
    }
    
    // Load sounds for a car
    if (!audio_load_sound1()) {
        return -1;
    }
    if (!audio_load_sound2()) {
        return -1;
    }
    
    // Start with sound1
    audio_switch_to_sound1();
    audio_start_loop();
    
    bool running = true;
    float speed = 1.0f;
    int current_sound = 1;
    
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_UP) {
                    speed += 0.1f;
                    printf("Speed: %.2f\n", speed);
                } else if (e.key.keysym.sym == SDLK_DOWN) {
                    speed -= 0.1f;
                    printf("Speed: %.2f\n", speed);
                } else if (e.key.keysym.sym == SDLK_1) {
                    audio_switch_to_sound1();
                    current_sound = 1;
                    printf("Switched to sound1\n");
                } else if (e.key.keysym.sym == SDLK_2) {
                    audio_switch_to_sound2();
                    current_sound = 2;
                    printf("Switched to sound2\n");
                }
            }
        }
        
        // Update audio speed every frame (no performance hit!)
        audio_set_speed(speed);
        audio_update();  // Optional, does nothing but exists for compatibility
        
        SDL_Delay(16);
    }
    
    audio_cleanup();
    SDL_Quit();
    
    return 0;
}