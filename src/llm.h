#pragma once

#include "http_client.h"

// Structure to hold LLM configuration
typedef struct {
    HttpClient* http_client;
    char* api_key;
    char* model;
    float temperature;
    int max_tokens;
} LLMConfig;

// Structure to hold a message
typedef struct {
    const char* role;    // "system", "user", or "assistant"
    const char* content; // The actual message content
} LLMMessage;

// Initialize LLM configuration
LLMConfig* llm_init(const char* api_key, const char* model);

// Free LLM configuration
void llm_cleanup(LLMConfig* config);

// Send a single message and get a response
char* llm_complete(LLMConfig* config, const char* prompt);

// Send multiple messages in a conversation
char* llm_chat(LLMConfig* config, const LLMMessage* messages, int message_count);

// Set temperature (0.0 to 1.0)
void llm_set_temperature(LLMConfig* config, float temperature);

// Set max tokens
void llm_set_max_tokens(LLMConfig* config, int max_tokens);
