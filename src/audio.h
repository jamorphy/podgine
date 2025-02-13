#pragma once

// Initialize/shutdown the audio system
void audio_init(void);
void audio_shutdown(void);

// Load an MP3 file and play it
void audio_play_file(const char* filepath);

// Stop current playback
void audio_stop(void);

// Check if audio is currently playing
bool audio_is_playing(void);
