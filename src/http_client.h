// http_client.h
#pragma once

#include <curl/curl.h>

typedef struct HttpClient {
    CURL *curl;
    // Add any additional client state here
} HttpClient;

typedef struct HttpResponse {
    char *data;
    size_t size;
    long status_code;
} HttpResponse;

// Core client functions
HttpClient* http_client_init(void);
void http_client_cleanup(HttpClient* client);

// Request functions
HttpResponse* http_client_post(
    HttpClient* client, 
    const char* url, 
    const char* headers[], 
    const char* body
);

// Response management
void http_response_free(HttpResponse* response);
