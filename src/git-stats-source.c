
#include "./git-stats-source.h"

#include <obs-internal.h>
#include <obs-module.h>
#include <obs-source.h>
#include <stdlib.h>

#include "./git-diff-interface.h"
#include "./hashMap/include/hashMap.h"
#include "hashMap/lib/include/untrackedFile.h"
#include "support.h"

// LOG_ERROR: for errors that don't require the program to exit
// LOG_WARNING: when error occurs and is recoverable
// LOG_INFO: info for whats going on
// LOG_DEBUG: use for debug //// only sent when debug is true

static void git_stats_update(void*, obs_data_t*);

static obs_properties_t* git_stats_properties(void*);

struct gitStatsInfo {
    // pointer to the text source
    obs_source_t* textSource;
    // pointer to the deletion text source
    obs_source_t* deletionSource;
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
    const char* text_source_id = "text_ft2_source";
    info->data = bzalloc(sizeof(struct gitData));
    info->data->trackedPaths = NULL;
    info->data->numTrackedFiles = 0;
    info->data->untracked = NULL;
    info->data->added = 0;
    info->data->deleted = 0;
    info->data->insertionEnabled = false;
    info->data->deletionEnabled = false;

    // the text source
    info->textSource =
        obs_source_create(text_source_id, "insertionSource", settings, NULL);
    obs_source_add_active_child(info->gitSource, info->textSource);
    info->deletionSource =
        obs_source_create(text_source_id, "deletionSource", NULL, NULL);
    obs_source_add_active_child(info->gitSource, info->deletionSource);

    git_stats_update(info, settings);
    obs_log(LOG_INFO, "Source Created");

    return (info);
}

// free up the source and its data
static void git_stats_destroy(void* data) {
    struct gitStatsInfo* info = data;

    obs_source_remove_active_child(info->gitSource, info->textSource);
    obs_source_remove_active_child(info->gitSource, info->deletionSource);
    info->textSource = NULL;
    info->deletionSource = NULL;

    bfree(info->data);
    info->data = NULL;

    bfree(info);

    obs_log(LOG_INFO, "Source Destroyed");
}

// get the width needed for the source
static uint32_t git_stats_width(void* data) {
    struct gitStatsInfo* info = data;

    return (
        obs_source_get_width(info->textSource) +
        obs_source_get_width(info->deletionSource));
}

// get the height needed for the source
static uint32_t git_stats_height(void* data) {
    struct gitStatsInfo* info = data;

    return (
        obs_source_get_height(info->textSource) +
        obs_source_get_height(info->deletionSource));
}

// set the settings defaults for the source
// TODO make defaults in general but specifically mkae the default colors have
// an alpha of 255 (so you can see it)
static void git_stats_get_defaults(obs_data_t* settings) {
    obs_data_set_default_bool(settings, "untracked_files", false);
    obs_data_set_default_int(settings, "delay", 5);
    // TODO do something for font
    // obs_data_set_default_obj();
    obs_data_set_default_int(settings, "deletion_color1", 0x00FF00);
    obs_data_set_default_int(settings, "deletion_color2", 0xFF0000);
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
    // static obs_properties_t* test = NULL;
    // test = git_stats_properties(info);
    // blah = obs_source_properties(info->deletionSource);
    //
    UNUSED_PARAMETER(data);

    // copy settings from dummy props to the deletion source
    obs_data_set_obj(
        info->deletionSource->context.settings, "font",
        obs_data_get_obj(settings, "deletion_font"));
    obs_data_set_bool(
        info->deletionSource->context.settings, "antialiasing",
        obs_data_get_bool(settings, "deletion_antialiasing"));
    obs_data_set_int(
        info->deletionSource->context.settings, "color1",
        obs_data_get_int(settings, "deletion_color1"));
    obs_data_set_int(
        info->deletionSource->context.settings, "color2",
        obs_data_get_int(settings, "deletion_color2"));
    obs_data_set_bool(
        info->deletionSource->context.settings, "outline",
        obs_data_get_bool(settings, "deletion_outline"));
    obs_data_set_bool(
        info->deletionSource->context.settings, "drop_shadow",
        obs_data_get_bool(settings, "deletion_drop_shadow"));

    char* allRepos = malloc(
        sizeof(char) * (strlen(obs_data_get_string(settings, "repos")) + 1));
    allRepos[0] = '\0';

    strcpy(allRepos, obs_data_get_string(settings, "repos"));

    if (!obs_data_get_bool(settings, "text_properties")) {
        info->data->insertionEnabled = false;
    }
    else {
        info->data->insertionEnabled = true;
    }

    if (!obs_data_get_bool(settings, "deletion_properties")) {
        info->data->deletionEnabled = false;
    }
    else {
        info->data->deletionEnabled = true;
    }

    if (allRepos == NULL || !strcmp(allRepos, "")) {
        obs_data_set_string(info->textSource->context.settings, "text", "");
        obs_data_set_string(info->deletionSource->context.settings, "text", "");
    }

    else {
        int amtHold = 0;
        info->data->trackedPaths = segmentString(allRepos, &amtHold);
        info->data->numTrackedFiles = amtHold;
    }

    if (strcmp(obs_data_get_string(settings, "repoDir"), "") &&
        (obs_data_get_string(settings, "repoDir") != NULL)) {
        addGitRepoDir(
            info->data, (char*)obs_data_get_string(settings, "repoDir"));
    }

    // do not have to do anything because it handles edge cases for me
    // (based on max and min) and doesn't allow empty input
    info->data->delayAmount = obs_data_get_int(settings, "delay");

    info->data->added = 0;
    info->data->deleted = 0;

    if (obs_data_get_bool(settings, "untracked_files")) {
        info->data->untracked = createHashMap();
        createUntrackedFilesHM(info->data);
    }

    if (!obs_data_get_bool(settings, "untracked_files") &&
        (info->data->untracked != NULL)) {
        info->data->untracked = createHashMap();
    }
}

// render out the source
static void git_stats_render(void* data, gs_effect_t* effect) {
    struct gitStatsInfo* info = data;
    // obs_source_video_render(info->deletionSource);
    obs_source_video_render(info->textSource);
    obs_source_video_render(info->deletionSource);
    UNUSED_PARAMETER(effect);
}

// updates the data (called each frame with the time elapsed passed in)
static void git_stats_tick(void* data, float seconds) {
    struct gitStatsInfo* info = data;
    if (!obs_source_showing(info->gitSource)) {
        return;
    }

    info->time_passed += seconds;
    if (info->time_passed > info->data->delayAmount) {
        info->time_passed = 0;
        if (info->data->trackedPaths == NULL) {
            obs_data_set_string(info->textSource->context.settings, "text", "");
            obs_source_update(
                info->textSource, info->textSource->context.settings);
            obs_data_set_string(
                info->deletionSource->context.settings, "text", "");
            obs_source_update(
                info->deletionSource, info->deletionSource->context.settings);
            return;
        }
        else {
            info->data->deleted = 0;
            info->data->added = 0;
            updateTrackedFiles(info->data);
        }
        if (info->data->untracked != NULL) {
            updateValueHM(&(info->data->untracked));
            info->data->added += getLinesAddedHM(&(info->data->untracked));
        }
        char outputBuffer[100] = "\0";
        if (info->data->insertionEnabled) {
            snprintf(
                outputBuffer, strlen(ltoa(info->data->added)) + 2, "+%s",
                ltoa(info->data->added));
            obs_data_set_string(
                info->textSource->context.settings, "text", outputBuffer);
            obs_source_update(
                info->textSource, info->textSource->context.settings);
        }
        else {
            snprintf(outputBuffer, strlen("") + 1, "%s", "");
            obs_data_set_string(
                info->textSource->context.settings, "text", outputBuffer);
            obs_source_update(
                info->textSource, info->textSource->context.settings);
        }
        strncpy(outputBuffer, "", 1);
        if (info->data->deletionEnabled) {
            snprintf(
                outputBuffer, strlen(ltoa(info->data->deleted)) + 2, "-%s",
                ltoa(info->data->deleted));
            obs_data_set_string(
                info->deletionSource->context.settings, "text", outputBuffer);
            obs_source_update(
                info->deletionSource, info->deletionSource->context.settings);
        }
        else {
            snprintf(outputBuffer, strlen("") + 1, "%s", "");
            obs_data_set_string(
                info->deletionSource->context.settings, "text", outputBuffer);
            obs_source_update(
                info->deletionSource, info->deletionSource->context.settings);
        }
    }
}

// what autogenerates the UI that I can get user data from
static obs_properties_t* git_stats_properties(void* unused) {
    struct gitStatsInfo* info = unused;
    UNUSED_PARAMETER(unused);
    obs_properties_t* props = obs_properties_create();

    obs_properties_t* repo_props = obs_properties_create();

    obs_properties_add_path(
        repo_props, "repoDir", "Directory Holding Repositories",
        OBS_PATH_DIRECTORY, NULL, NULL);

    obs_properties_add_text(
        repo_props, "repos", "Repositiories", OBS_TEXT_MULTILINE);

    obs_properties_add_int(
        repo_props, "delay", "Delay Between Updates", 0, INT_MAX, 1);

    obs_properties_add_bool(
        repo_props, "untracked_files", "Account For Untracked Files");

    obs_properties_add_group(
        props, "repo_properties", "Repository Settings", OBS_GROUP_NORMAL,
        repo_props);

    ///////////////

    obs_properties_t* text1_props = obs_source_properties(info->textSource);
    obs_properties_remove_by_name(text1_props, "text_file");
    obs_properties_remove_by_name(text1_props, "from_file");
    obs_properties_remove_by_name(text1_props, "log_mode");
    obs_properties_remove_by_name(text1_props, "log_lines");
    obs_properties_remove_by_name(text1_props, "word_wrap");
    obs_properties_remove_by_name(text1_props, "text");
    obs_properties_remove_by_name(text1_props, "custom_width");

    obs_properties_add_group(
        props, "text_properties", "Text Settings", OBS_GROUP_CHECKABLE,
        text1_props);

    //////////////

    // TODO research these functions and use them to one by one add the
    // properties with a changed name
    //  obs_properties_get: get a property by name from the textsource
    //
    // obs_properties_t* text2_props =
    // obs_source_properties(info->deletionSource);

    obs_properties_t* text2_props = obs_properties_create();
    obs_properties_add_font(text2_props, "deletion_font", "font");
    obs_properties_add_bool(
        text2_props, "deletion_antialiasing", "antialiasing");
    obs_properties_add_color(text2_props, "deletion_color1", "Color1");
    obs_properties_add_color_alpha(text2_props, "deletion_color2", "Color2");

    obs_properties_add_bool(text2_props, "deletion_outline", "Outline");

    obs_properties_add_bool(text2_props, "deletion_drop_shadow", "Drop Shadow");

    /*obs_property_t* ptr = obs_properties_first(text2_props);*/
    /*bool checker = ptr ? true : false;*/
    /*while (checker) {*/
    /*    enum obs_property_type type = obs_property_get_type(ptr);*/
    /*    printf("TEST: %d\n", type);*/
    /*    // TODO look at line 535 of obs-properties.c in obs source code this
     * is*/
    /*    // where I'm thinking the data is stored and what we actullay need
     * to*/
    /*    // link for data*/
    /*    obs_data_t* data = get_property_data(ptr);*/
    /*    checker = obs_property_next(&ptr);*/
    /*}*/

    obs_properties_add_group(
        props, "deletion_properties", "Deletion Settings", OBS_GROUP_CHECKABLE,
        text2_props);

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
