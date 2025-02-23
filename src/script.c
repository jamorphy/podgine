#include <stdlib.h>
#include <sys/dir.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "cJSON.h"

#include "log.h"
#include "types.h"
#include "script.h"
#include "utils.h"
#include "audio.h"
#include "render.h"
#include "camera.h"

void init_script_pipe(void) {
    if (access("engine_pipe", F_OK) == -1) {
        mkfifo("engine_pipe", 0666);
    }
}

void check_script_pipe(World* world) {
    static time_t last_check = 0;
    static int fd = -1;
    time_t current = time(NULL);
    
    if (current - last_check < 3) return;
    last_check = current;

    if (fd == -1) {
        fd = open("engine_pipe", O_RDONLY | O_NONBLOCK);
        if (fd == -1) {
            LOG_WARN("Failed to open script listen pipe");
            return;
        }
    }
    
    char path[256] = {0};
    ssize_t bytes = read(fd, path, sizeof(path)-1);
    
    if (bytes > 0) {
        path[strcspn(path, "\n")] = 0;
        add_script(&world->script_queue, path);
        LOG_DEBUG("Added %s to script queue", path);
    }
}

void add_script(ScriptQueue* queue, const char* filepath) {
    queue->scripts[queue->tail] = strdup(filepath);
    queue->tail = (queue->tail + 1) % 100;
    queue->size++;
}

// Get and remove next script
char* get_next_script(ScriptQueue* queue) {
    if (queue->size == 0) return NULL;
    char* script = queue->scripts[queue->head];
    queue->head = (queue->head + 1) % 100;
    queue->size--;
    return script;
}

void cleanup_script(Script* script)
{
    if (script) {
        for (int i = 0; i < script->line_count; i++) {
            free(script->lines[i].character);
            free(script->lines[i].text);
            free(script->lines[i].audio_file);
        }
        free(script->lines);
        script->line_count = 0;
    }
}

void load_script(World* world, const char* filepath)
{
    char* script_text = read_text_file(filepath);
    if (!script_text) {
        LOG_WARN("Cannot read script");
    }
    cJSON* json = cJSON_Parse(script_text);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            LOG_WARN("Error parsing JSON: %s\n", error_ptr);
        }
    }

    world->active_script = malloc(sizeof(Script));
    world->active_script->lines = NULL;
    world->active_script->line_count = 0;

    cJSON *dialogue = cJSON_GetObjectItem(json, "dialogue");
    if (!dialogue) {
        printf("Error: No dialogue array found in JSON\n");
        return;
    }

    int dialogue_count = cJSON_GetArraySize(dialogue);
    world->active_script->lines = (Line*)malloc(sizeof(Line) * dialogue_count);
    world->active_script->line_count = dialogue_count;


    for (int i = 0; i < dialogue_count; i++) {
        cJSON *dialogue_item = cJSON_GetArrayItem(dialogue, i);
        world->active_script->lines[i].character = strdup(cJSON_GetObjectItem(dialogue_item, "character")->valuestring);
        world->active_script->lines[i].text = strdup(cJSON_GetObjectItem(dialogue_item, "text")->valuestring);
        world->active_script->lines[i].audio_file = strdup(cJSON_GetObjectItem(dialogue_item, "audio_file")->valuestring);
    }
    world->is_script_active = true;
    cJSON_Delete(json);
    free(script_text);
}

void cleanup_active_script(World* world) {
    if (world->active_script) {
        cleanup_script(world->active_script);
        free(world->active_script);
        world->active_script = NULL;
    }
}

void play_next_line(World* world) {
    if (!world->active_script) return;

    if (!world->is_playing_audio &&
        world->active_script->current_line < world->active_script->line_count) {

        audio_play_file(world->active_script->lines[world->active_script->current_line].audio_file);
        world->is_playing_audio = true;

        const char* current_character = world->active_script->lines[world->active_script->current_line].character;
        if (world->active_script->current_speaking_character == NULL ||
            strcmp(world->active_script->current_speaking_character, current_character) != 0) {

            switch_to_character_camera(world, current_character);

            if (world->active_script->current_speaking_character) {
                free(world->active_script->current_speaking_character);
            }

            world->active_script->current_speaking_character = strdup(current_character);
        }
    }

    if (world->is_playing_audio) {

        render_text(1.0f, 55.0f, world->active_script->lines[world->active_script->current_line].text);

        if (!audio_is_playing()) {
            world->is_playing_audio = false;
            world->active_script->current_line++;

            // Check if script is finished
            if (world->is_script_active &&
                world->active_script->current_line >= world->active_script->line_count) {
                LOG_DEBUG("Script finished");
                world->is_script_active = false;
                free(world->active_script);

                char* next_script = get_next_script(&world->script_queue);
                if (next_script) {
                    load_script(world, next_script);
                    free(next_script);
                }
            }
        }
    }
}
