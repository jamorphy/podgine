#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define SOKOL_AUDIO_IMPL
#include "../libs/sokol/sokol_audio.h"
#define DR_MP3_IMPLEMENTATION
#include "../libs/dr_mp3.h"
#include "audio.h"

static struct {
    float* audio_buffer;
    int num_samples;
    int current_sample;
    bool is_playing;
} state = {0};

static void audio_stream_callback(float* buffer, int num_frames, int num_channels) {
    // First verify we're actually getting called with data
    static bool first_call = true;
    if (first_call) {
        printf("First callback: frames=%d, channels=%d\n", num_frames, num_channels);
        first_call = false;
    }

    if (!state.is_playing || !state.audio_buffer) {
        // Fill with silence
        for (int i = 0; i < num_frames * num_channels; i++) {
            buffer[i] = 0.0f;
        }
        return;
    }

    // Copy frames to output buffer
    for (int i = 0; i < num_frames * num_channels; i++) {
        if (state.current_sample < state.num_samples) {
            buffer[i] = state.audio_buffer[state.current_sample++] * 0.8;
        } else {
            buffer[i] = 0.0f;
            if (i == 0) {
                state.is_playing = false;
                printf("Reached end of audio data\n");
            }
        }
    }
}

void audio_init(void) {
    // Match common MP3 format
    saudio_setup(&(saudio_desc){
            .stream_cb = audio_stream_callback,
            .num_channels = 1,     // Changed to mono since your MP3 is mono
            .sample_rate = 44100,  // Match MP3 sample rate
            .buffer_frames = 2048  // Add explicit buffer size
        });
    
    if (!saudio_isvalid()) {
        printf("Failed to initialize audio system!\n");
        return;
    }
    printf("Audio system initialized: %d Hz, %d channels\n", 
           saudio_sample_rate(), saudio_channels());
}

void audio_play_file(const char* filepath) {
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, filepath, NULL)) {
        printf("Failed to load MP3\n");
        return;
    }

    drmp3_uint64 total_frame_count = drmp3_get_pcm_frame_count(&mp3);
    state.num_samples = (int)(total_frame_count * mp3.channels);
    printf("Loading MP3: pcm_frames=%llu (%.2f seconds)\n", 
           total_frame_count, 
           (float)total_frame_count / mp3.sampleRate);

    state.audio_buffer = (float*)malloc(sizeof(float) * state.num_samples);
    if (!state.audio_buffer) {
        drmp3_uninit(&mp3);
        printf("Failed to allocate audio buffer\n");
        return;
    }

    drmp3_read_pcm_frames_f32(&mp3, total_frame_count, state.audio_buffer);
    drmp3_uninit(&mp3);

    state.current_sample = 0;
    state.is_playing = true;
}

void audio_stop(void) {
    state.is_playing = false;
    state.current_sample = 0;
    if (state.audio_buffer) {
        free(state.audio_buffer);
        state.audio_buffer = NULL;
    }
}

void audio_shutdown(void) {
    audio_stop();
    saudio_shutdown();
}

bool audio_is_playing(void) {
    return state.is_playing;
}
