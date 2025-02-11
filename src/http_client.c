// http_client.c
#include "http_client.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    HttpResponse *response = (HttpResponse *)userp;
    
    char *ptr = realloc(response->data, response->size + realsize + 1);
    if(!ptr) {
        fprintf(stderr, "Failed to allocate memory!\n");
        return 0;
    }
    
    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, realsize);
    response->size += realsize;
    response->data[response->size] = 0;
    
    return realsize;
}

HttpClient* http_client_init(void) {
    HttpClient* client = malloc(sizeof(HttpClient));
    if (!client) {
        return NULL;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    client->curl = curl_easy_init();
    
    if (!client->curl) {
        free(client);
        return NULL;
    }

    return client;
}

void http_client_cleanup(HttpClient* client) {
    if (client) {
        if (client->curl) {
            curl_easy_cleanup(client->curl);
        }
        free(client);
    }
    curl_global_cleanup();
}

HttpResponse* http_client_post(
    HttpClient* client, 
    const char* url, 
    const char* headers[], 
    const char* body
) {
    if (!client || !client->curl) {
        return NULL;
    }

    HttpResponse* response = malloc(sizeof(HttpResponse));
    if (!response) {
        return NULL;
    }

    // Initialize response structure
    response->data = malloc(1);
    response->size = 0;
    response->status_code = 0;

    // Reset CURL handle
    curl_easy_reset(client->curl);

    // Set up headers
    struct curl_slist *header_list = NULL;
    if (headers) {
        for (int i = 0; headers[i] != NULL; i++) {
            header_list = curl_slist_append(header_list, headers[i]);
        }
    }

    // Set CURL options
    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, (void *)response);

    // Perform request
    CURLcode res = curl_easy_perform(client->curl);

    // Get HTTP status code
    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, &response->status_code);

    // Cleanup headers
    if (header_list) {
        curl_slist_free_all(header_list);
    }

    // Handle errors
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        http_response_free(response);
        return NULL;
    }

    return response;
}

void http_response_free(HttpResponse* response) {
    if (response) {
        if (response->data) {
            free(response->data);
        }
        free(response);
    }
}

// Example usage function
HttpResponse* http_client_test_request(HttpClient* client) {
    if (!client || !client->curl) {
        return NULL;
    }

    HttpResponse* response = malloc(sizeof(HttpResponse));
    if (!response) {
        return NULL;
    }

    // Initialize response structure
    response->data = malloc(1);
    response->size = 0;
    response->status_code = 0;

    // Reset CURL handle
    curl_easy_reset(client->curl);

    // Test endpoint - gets a sample post from JSONPlaceholder
    const char* test_url = "https://jsonplaceholder.typicode.com/posts/8";

    // Set CURL options
    curl_easy_setopt(client->curl, CURLOPT_URL, test_url);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, (void *)response);

    // Perform request
    CURLcode res = curl_easy_perform(client->curl);

    // Get HTTP status code
    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, &response->status_code);

    // Handle errors
    if (res != CURLE_OK) {
        fprintf(stderr, "Test request failed: %s\n", curl_easy_strerror(res));
        http_response_free(response);
        return NULL;
    }

    // Log the raw output
    printf("\n=== Test Request Results ===\n");
    printf("Status Code: %ld\n", response->status_code);
    printf("Raw Response:\n%s\n", response->data);
    printf("========================\n\n");

    return response;
}

void example_usage(void) {
    HttpClient* client = http_client_init();
    if (!client) {
        fprintf(stderr, "Failed to initialize HTTP client\n");
        return;
    }

    // First make the test request
    printf("Making test request to JSONPlaceholder...\n");
    HttpResponse* test_response = http_client_test_request(client);
    if (test_response) {
        http_response_free(test_response);
    }

    // Then show the normal request
    const char* headers[] = {
        "Content-Type: application/json",
        "Authorization: Bearer your-api-key",
        NULL
    };

    const char* body = "{\"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \"Hello!\"}]}";

    HttpResponse* response = http_client_post(
        client,
        "https://api.openai.com/v1/chat/completions",
        headers,
        body
    );

    if (response) {
        printf("Status Code: %ld\n", response->status_code);
        printf("Response: %s\n", response->data);
        http_response_free(response);
    }

    http_client_cleanup(client);
}

/* int main() { */
/*     HttpClient* client = http_client_init(); */
/*     if (!client) { */
/*         fprintf(stderr, "Failed to initialize HTTP client\n"); */
/*         return 1; */
/*     } */

/*     HttpResponse* response = http_client_test_request(client); */
/*     if (response) { */
/*         http_response_free(response); */
/*     } */

/*     http_client_cleanup(client); */
/*     return 0; */
/* } */
