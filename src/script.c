#include <stdlib.h>
#include <sys/dir.h>
#include <unistd.h>
#include "cJSON.h"

#include "types.h"
#include "script.h"
#include "utils.h"
#include "audio.h"
#include "render.h"
#include "camera.h"

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
    printf("I'm in load script heres the filepath: %s\n", filepath);
    char* script_text = read_text_file(filepath);
    if (!script_text) {
        // TODO: fix
        printf("cant read script gg\n");
    }
    cJSON* json = cJSON_Parse(script_text);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error parsing JSON: %s\n", error_ptr);
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
    printf("Found %d dialogue entries\n", dialogue_count);

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
    if (!world->active_script) {
        printf("No active script, mallocing it\n");
        return;
    }

    if (!world->is_playing_audio &&
        world->active_script->current_line < world->active_script->line_count) {

        audio_play_file(world->active_script->lines[world->active_script->current_line].audio_file);
        world->is_playing_audio = true;

        const char* current_character = world->active_script->lines[world->active_script->current_line].character;
        if (world->active_script->current_speaking_character == NULL ||
            strcmp(world->active_script->current_speaking_character, current_character) != 0) {

            printf("%s is speaking.\n", current_character);
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
            if (world->active_script->current_line >= world->active_script->line_count) {
                printf("Script finished\n");
                world->is_script_active = false;
                free(world->active_script);

                world->backlog_i++;

                if (world->backlog[world->backlog_i]) {
                    printf("about to queue the next script at index %i\n", world->backlog_i);
                    load_script(world, world->backlog[world->backlog_i]);
                }
            }
        }
    }
}
