#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define SOKOL_AUDIO_IMPL
#include "../libs/sokol/sokol_audio.h"
#define DR_MP3_IMPLEMENTATION
#include "../libs/dr_mp3.h"
#include "log.h"
#include "audio.h"

static struct {
    float* audio_buffer;
    int num_samples;
    int current_sample;
    bool is_playing;
} state = {0};

static void audio_stream_callback(float* buffer, int num_frames, int num_channels) {

    static bool first_call = true;
    if (first_call) {
        first_call = false;
    }

    if (!state.is_playing || !state.audio_buffer) {
        for (int i = 0; i < num_frames * num_channels; i++) {
            buffer[i] = 0.0f;
        }
        return;
    }

    // Copy frames to output buffer
    for (int i = 0; i < num_frames * num_channels; i++) {
        if (state.current_sample < state.num_samples) {
            buffer[i] = state.audio_buffer[state.current_sample++];
        } else {
            buffer[i] = 0.0f;
            if (i == 0) {
                state.is_playing = false;
            }
        }
    }
}

void audio_init(void) {
    // Match common MP3 format
    saudio_setup(&(saudio_desc){
            .stream_cb = audio_stream_callback,
            .num_channels = 1,
            .sample_rate = 24000,  // elevenlabs 44100 Kokoro 24000
            .buffer_frames = 2048
        });
    
    if (!saudio_isvalid()) {
        LOG_FATAL("Failed to initialize audio system!");
        return;
    }
    LOG_DEBUG("Initialized audio system: %d Hz, %d channels", 
           saudio_sample_rate(), saudio_channels());
}

void audio_play_file(const char* filepath) {
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, filepath, NULL)) {
        LOG_WARN("Failed to load MP3");
        return;
    }

    drmp3_uint64 total_frame_count = drmp3_get_pcm_frame_count(&mp3);
    state.num_samples = (int)(total_frame_count * mp3.channels);
    /* LOG_DEBUG("Loading MP3: pcm_frames=%llu (%.2f seconds)",  */
    /*        total_frame_count,  */
    /*        (float)total_frame_count / mp3.sampleRate); */

    state.audio_buffer = (float*)malloc(sizeof(float) * state.num_samples);
    if (!state.audio_buffer) {
        drmp3_uninit(&mp3);
        LOG_WARN("Failed to allocate audio buffer");
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
