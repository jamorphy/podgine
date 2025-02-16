#include <stdlib.h>
#include "cJSON.h"

#include "types.h"
#include "script.h"
#include "utils.h"
#include "audio.h"
#include "render.h"
#include "camera.h"

void parse_script(World* world, cJSON *json)
{
    // Allocate the script if it doesn't exist
    if (!world->script) {
        world->script = (Script*)malloc(sizeof(Script));
    }

    // Get the dialogue array
    cJSON *dialogue = cJSON_GetObjectItem(json, "dialogue");
    
    if (dialogue) {
        int dialogue_count = cJSON_GetArraySize(dialogue);
        printf("Found %d dialogue entries\n", dialogue_count);
        
        // Allocate space for all lines
        world->script->lines = (Line*)malloc(sizeof(Line) * dialogue_count);
        world->script->line_count = dialogue_count;
        
        // Parse each dialogue entry
        for (int i = 0; i < dialogue_count; i++) {
            cJSON *dialogue_item = cJSON_GetArrayItem(dialogue, i);
            
            world->script->lines[i].character = strdup(cJSON_GetObjectItem(dialogue_item, "character")->valuestring);
            world->script->lines[i].text = strdup(cJSON_GetObjectItem(dialogue_item, "text")->valuestring);
            world->script->lines[i].audio_file = strdup(cJSON_GetObjectItem(dialogue_item, "audio_file")->valuestring);
        }
    }
}

void play_script(World* world)
{
    char* script_text = read_text_file("api/generated/podcast_1739697648/script.json");
    if (!script_text) {
        // TODO: fix
        printf("cant read script gg\n");
    } else {
        printf("hers the cript: %s\n", script_text);
    }
    cJSON* json = cJSON_Parse(script_text);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error parsing JSON: %s\n", error_ptr);
        }
    } else {
        // Debug: Check if we can access dialogue array
        cJSON* dialogue = cJSON_GetObjectItem(json, "dialogue");
        if (dialogue) {
            int count = cJSON_GetArraySize(dialogue);
            printf("Found dialogue array with %d items\n", count);
            
            // Debug: Try to access first item
            cJSON* first_item = cJSON_GetArrayItem(dialogue, 0);
            if (first_item) {
                cJSON* char_item = cJSON_GetObjectItem(first_item, "character");
                if (char_item) {
                    printf("First character: %s\n", char_item->valuestring);
                }
            }
        }

        parse_script(world, json);
        cJSON_Delete(json);
        free(script_text);
    }
}


// TODO: clean up when script is fully finished
void play_next_line(World* world) {
    if (!world->is_playing_audio && 
        world->current_line < world->script->line_count) {
        
        // Start playing the current line
        audio_play_file(world->script->lines[world->current_line].audio_file);
        world->is_playing_audio = true;
        
        // Switch camera only when starting a new line with a different character
        const char* current_character = world->script->lines[world->current_line].character;
        if (world->current_speaking_character == NULL || 
            strcmp(world->current_speaking_character, current_character) != 0) {
            
            printf("%s is speaking.\n", current_character);
            switch_to_character_camera(world, current_character);
            
            // Update current speaking character
            if (world->current_speaking_character) {
                free(world->current_speaking_character);
            }
            world->current_speaking_character = strdup(current_character);
        }
    }
    
    // Check if current audio finished
    if (world->is_playing_audio) {
        render_text(1.0f, 57.0f, world->script->lines[world->current_line].text);
        
        if (!audio_is_playing()) {
            // Audio finished, move to next line
            world->is_playing_audio = false;
            world->current_line++;
        }
    }
}

