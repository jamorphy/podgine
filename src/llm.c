#include "llm.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPENAI_API_URL "https://api.openai.com/v1/chat/completions"

LLMConfig* llm_init(const char* api_key, const char* model) {
    LLMConfig* config = malloc(sizeof(LLMConfig));
    if (!config) return NULL;

    config->http_client = http_client_init();
    if (!config->http_client) {
        free(config);
        return NULL;
    }

    config->api_key = strdup(api_key);
    config->model = strdup(model);
    config->temperature = 0.7;
    config->max_tokens = 1000;

    return config;
}

void llm_set_temperature(LLMConfig* config, float temperature) {
    if (config) {
        config->temperature = temperature;
    }
}

void llm_set_max_tokens(LLMConfig* config, int max_tokens) {
    if (config) {
        config->max_tokens = max_tokens;
    }
}

void llm_cleanup(LLMConfig* config) {
    if (config) {
        if (config->http_client) {
            http_client_cleanup(config->http_client);
        }
        free(config->api_key);
        free(config->model);
        free(config);
    }
}

static char* create_chat_request_body(LLMConfig* config, const LLMMessage* messages, int message_count) {
    cJSON *root = cJSON_CreateObject();
    if (!root) return NULL;

    // Add model
    cJSON_AddStringToObject(root, "model", config->model);
    
    // Add temperature
    cJSON_AddNumberToObject(root, "temperature", config->temperature);
    
    // Add max_tokens
    cJSON_AddNumberToObject(root, "max_tokens", config->max_tokens);
    
    // Create messages array
    cJSON *messages_array = cJSON_AddArrayToObject(root, "messages");
    if (!messages_array) {
        cJSON_Delete(root);
        return NULL;
    }

    // Add each message to the array
    for (int i = 0; i < message_count; i++) {
        cJSON *message = cJSON_CreateObject();
        if (!message) {
            cJSON_Delete(root);
            return NULL;
        }
        
        cJSON_AddStringToObject(message, "role", messages[i].role);
        cJSON_AddStringToObject(message, "content", messages[i].content);
        cJSON_AddItemToArray(messages_array, message);
    }

    // Convert to string
    char *body = cJSON_Print(root);
    cJSON_Delete(root);
    
    return body;
}

static char* extract_response_content(const char* json_str) {
    char* result = NULL;
    cJSON *root = cJSON_Parse(json_str);
    if (!root) return NULL;

    // Get choices array
    cJSON *choices = cJSON_GetObjectItem(root, "choices");
    if (!choices || !cJSON_IsArray(choices) || cJSON_GetArraySize(choices) == 0) {
        cJSON_Delete(root);
        return NULL;
    }

    // Get first choice
    cJSON *first_choice = cJSON_GetArrayItem(choices, 0);
    if (!first_choice) {
        cJSON_Delete(root);
        return NULL;
    }

    // Get message
    cJSON *message = cJSON_GetObjectItem(first_choice, "message");
    if (!message) {
        cJSON_Delete(root);
        return NULL;
    }

    // Get content
    cJSON *content = cJSON_GetObjectItem(message, "content");
    if (content && cJSON_IsString(content) && content->valuestring) {
        result = strdup(content->valuestring);
    }

    cJSON_Delete(root);
    return result;
}

char* llm_complete(LLMConfig* config, const char* prompt) {
    LLMMessage message = {"user", prompt};
    return llm_chat(config, &message, 1);
}

char* llm_chat(LLMConfig* config, const LLMMessage* messages, int message_count) {
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", config->api_key);

    const char* headers[] = {
        "Content-Type: application/json",
        auth_header,
        NULL
    };

    char* body = create_chat_request_body(config, messages, message_count);
    if (!body) return NULL;

    HttpResponse* response = http_client_post(
        config->http_client,
        OPENAI_API_URL,
        headers,
        body
    );
    free(body);

    if (!response) return NULL;

    char* content = NULL;
    if (response->status_code == 200) {
        content = extract_response_content(response->data);
    } else {
        fprintf(stderr, "API request failed with status code: %ld\n", response->status_code);
        fprintf(stderr, "Response: %s\n", response->data);
    }

    http_response_free(response);
    return content;
}

/* int main() { */
/*     LLMConfig* config = llm_init("apikeyhere", "gpt-4o-turbo"); */
/*     if (!config) { */
/*         fprintf(stderr, "Failed to initialize LLM\n"); */
/*         return 1; */
/*     } */

/*     llm_set_temperature(config, 0.7); */
/*     llm_set_max_tokens(config, 1000); */

/*     char* response = llm_complete(config, "Tell me a deez nuts joke."); */
/*     if (response) { */
/*         printf("Response: %s\n", response); */
/*         free(response); */
/*     } */

/*     llm_cleanup(config); */
/*     return 0; */
/* } */
