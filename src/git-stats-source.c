
#include "./git-stats-source.h"

#include <obs-internal.h>
#include <obs-module.h>
#include <obs-source.h>
#include <stdlib.h>

#include "./git-diff-interface.h"
#include "./hashMap/include/hashMap.h"
#include "hashMap/lib/include/untrackedFile.h"
#include "support.h"
int MAXNUMPATHS = 25;

// LOG_ERROR: for errors that don't require the program to exit
// LOG_WARNING: when error occurs and is recoverable
// LOG_INFO: info for whats going on
// LOG_DEBUG: use for debug //// only sent when debug is true
// for debugging general issues
#define DEBUG_LOG 0
// for debugging seg faults
#define DEBUG_PRINT 0
struct gitStatsInfo {
    // pointer to the text source
    obs_source_t* textSource;
    // pointer to our source!!
    obs_source_t* gitSource;
    // length of the text source
    uint32_t cx;
    // height of the text source
    uint32_t cy;
    // time passed between the updates
    float time_passed;

    // the information that we get from our git stats thingy madonker
    struct gitData* data;
};

static const char* git_stats_name(void* unused) {
    UNUSED_PARAMETER(unused);
    return (obs_module_text("GitStats"));
}

// create data for the source
static void* git_stats_create(obs_data_t* settings, obs_source_t* source) {
    struct gitStatsInfo* info = bzalloc(sizeof(struct gitStatsInfo));
    info->time_passed = 0;
    // the source itself
    info->gitSource = source;

    // id for the text source
    const char* text_source_id = "text_ft2_source\0";

    info->data = bzalloc(sizeof(struct gitData));
    info->data->trackedPaths = NULL;
    info->data->numTrackedFiles = 0;
    info->data->untracked = createHashMap();
    info->data->added = 0;
    info->data->deleted = 0;

    // the text source
    info->textSource =
        obs_source_create(text_source_id, text_source_id, settings, NULL);
    obs_source_add_active_child(info->gitSource, info->textSource);

    obs_log(LOG_INFO, "Source Created");

    return (info);
}

// free up the source and its data
static void git_stats_destroy(void* data) {
    struct gitStatsInfo* info = data;

    obs_source_remove(info->textSource);
    obs_source_release(info->textSource);
    info->textSource = NULL;

    bfree(info->data);
    info->data = NULL;

    bfree(info);

    obs_log(LOG_INFO, "Source Destroyed");
}

// get the width needed for the source
static uint32_t git_stats_width(void* data) {
    struct gitStatsInfo* info = data;

    return (obs_source_get_width(info->textSource));
}

// get the height needed for the source
static uint32_t git_stats_height(void* data) {
    struct gitStatsInfo* info = data;

    return (obs_source_get_height(info->textSource));
}

// set the settings defaults for the source
static void git_stats_get_defaults(obs_data_t* settings) {
    obs_data_set_default_bool(settings, "untracked_files", false);
    obs_data_set_default_string(settings, "delay", "5");
}

// takes string and delimits it by newline chars
char** segmentString(char* string, int* numPaths) {
    if (string == NULL) {
        return (NULL);
        *numPaths = 0;
    }
    char* tmpString = malloc(sizeof(char) * (strlen(string) + 1));
    tmpString[0] = '\0';
    strcpy(tmpString, string);
    char* buffer;
    char** paths = malloc(sizeof(char*) * MAXNUMPATHS);
    buffer = strtok(tmpString, "\n");
    int count = 0;
    while (buffer != NULL) {
        paths[count] = malloc(sizeof(char) * (strlen(buffer) + 1));
        strcpy(paths[count], buffer);
        count++;
        buffer = strtok(NULL, "\n");
    }
    for (int i = count; i < MAXNUMPATHS; i++) {
        paths[i] = NULL;
    }

    *numPaths = count;
    return (paths);
}

// this runs when you update settings
static void git_stats_update(void* data, obs_data_t* settings) {
    struct gitStatsInfo* info = data;
    UNUSED_PARAMETER(data);

    char* allRepos = malloc(
        sizeof(char) * (strlen(obs_data_get_string(settings, "repos")) + 1));
    allRepos[0] = '\0';

    strcpy(allRepos, obs_data_get_string(settings, "repos"));

    if (allRepos == NULL) {
        obs_data_set_string(info->textSource->context.settings, "text", "");
    }

    else {
        int amtHold = 0;
        info->data->trackedPaths = segmentString(allRepos, &amtHold);
        info->data->numTrackedFiles = amtHold;
    }

    if (!strcmp("", obs_data_get_string(settings, "delay"))) {
        info->data->delayAmount = 5;
    }
    else {
        info->data->delayAmount = atoi(obs_data_get_string(settings, "delay"));
    }

    updateTrackedFiles(info->data);
    info->data->added = 0;
    info->data->deleted = 0;
    if (obs_data_get_bool(settings, "untracked_files")) {
        createUntrackedFilesHM(info->data);
        updateValueHM(&(info->data->untracked));
        info->data->added += getLinesAddedHM(&(info->data->untracked));
    }
}

// render out the source
static void git_stats_render(void* data, gs_effect_t* effect) {
    struct gitStatsInfo* info = data;
    obs_source_video_render(info->textSource);
    UNUSED_PARAMETER(effect);
}

// updates the data (called each frame with the time elapsed passed in)
static void git_stats_tick(void* data, float seconds) {
    struct gitStatsInfo* info = data;
    // don't update if the source isn't active
    if (!obs_source_showing(info->gitSource)) {
        return;
    }

    info->time_passed += seconds;
    if (info->data->trackedPaths == NULL) {
        obs_data_set_string(info->textSource->context.settings, "text", "");
        obs_source_update(info->textSource, info->textSource->context.settings);
        return;
    }
    else {
        info->data->deleted = 0;
        info->data->added = 0;
        updateTrackedFiles(info->data);
    }
    if (info->data->untracked != NULL) {
        updateValueHM(&(info->data->untracked));
        // printHM(info->data->untracked);
    }
    obs_data_set_string(
        info->textSource->context.settings, "text", ltoa(info->data->added));

    obs_source_update(info->textSource, info->textSource->context.settings);
}

// what autogenerates the UI that I can get user data from (learn about this)
static obs_properties_t* git_stats_properties(void* unused) {
    struct gitStatsInfo* info = unused;
    UNUSED_PARAMETER(unused);
    obs_properties_t* props = obs_source_properties(info->textSource);

    // remove certain properties from the freetype text source
    obs_properties_remove_by_name(props, "text");
    obs_properties_remove_by_name(props, "text_file");
    obs_properties_remove_by_name(props, "from_file");
    obs_properties_remove_by_name(props, "log_mode");
    obs_properties_remove_by_name(props, "log_lines");
    obs_properties_remove_by_name(props, "word_wrap");

    obs_properties_add_text(
        props, "repos", "Repositiories", OBS_TEXT_MULTILINE);

    obs_properties_add_text(
        props, "delay", "Delay Between Updates", OBS_TEXT_DEFAULT);

    obs_properties_add_bool(
        props, "untracked_files", "Account For Untracked Files");

    return props;
}

// clang-format off
    struct obs_source_info git_stats_source = {
        .id           = "git-stats",
        .type         = OBS_SOURCE_TYPE_INPUT,
        .output_flags = OBS_SOURCE_VIDEO,
        //get the name of the source
        .get_name     = git_stats_name,
        //create the internal structures needed for the source
        .create       = git_stats_create,
        .destroy      = git_stats_destroy,
        .update       = git_stats_update,
        .video_render = git_stats_render,
        .get_width    = git_stats_width,
        .get_height   = git_stats_height,
        .video_tick = git_stats_tick, 
        .get_properties = git_stats_properties
    };
// clang-format on
