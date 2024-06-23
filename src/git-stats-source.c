
#include "./git-stats-source.h"

#include <obs-internal.h>
#include <obs-module.h>
#include <obs-source.h>
#include <stdlib.h>

#include "./git-diff-interface.h"
#include "./hashMap/include/hashMap.h"
#include "hashMap/lib/include/untrackedFile.h"
#include "support.h"

#define OVERLOAD_VAL 9999

static bool testMode = false;

static bool INIT_RUN = true;

#define INVAL_TIME_THRESH 10

// LOG_ERROR: for errors that don't require the program to exit
// LOG_WARNING: when error occurs and is recoverable
// LOG_INFO: info for whats going on
// LOG_DEBUG: use for debug //// only sent when debug is true

static void git_stats_update(void*, obs_data_t*);

static void git_stats_get_defaults(obs_data_t*);

// static bool plugin_created = false;
static obs_properties_t* git_stats_properties(void*);
struct gitStatsInfo {
    obs_source_t* insertionSource;
    // pointer to the text source
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

    // time passed between prior invalid check
    float invalid_time_passed;

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
    info->invalid_time_passed = 0;
    // the source itself
    info->gitSource = source;

    // id for the text source
    const char* text_source_id = "text_ft2_source_v2";
    info->data = bzalloc(sizeof(struct gitData));
    info->data->trackedPaths = NULL;
    info->data->numTrackedFiles = 0;
    info->data->untracked = NULL;
    info->data->added = 0;
    info->data->deleted = 0;
    info->data->insertionEnabled = false;
    info->data->deletionEnabled = false;

    // the text source
    info->insertionSource =
        obs_source_create(text_source_id, "insertionSource", settings, NULL);
    obs_source_add_active_child(info->gitSource, info->insertionSource);
    info->deletionSource =
        obs_source_create(text_source_id, "deletionSource", NULL, NULL);
    obs_source_add_active_child(info->gitSource, info->deletionSource);

    // ensure that defaults are set AFTER the creation has completed
    git_stats_get_defaults(settings);

    git_stats_update(info, settings);
    obs_log(LOG_INFO, "Source Created");

    return (info);
}

// free up the source and its data
static void git_stats_destroy(void* data) {
    struct gitStatsInfo* info = data;

    obs_source_remove_active_child(info->gitSource, info->insertionSource);
    obs_source_remove_active_child(info->gitSource, info->deletionSource);

    obs_source_remove(info->insertionSource);
    obs_source_release(info->insertionSource);
    info->insertionSource = NULL;

    obs_source_remove(info->deletionSource);
    obs_source_release(info->deletionSource);
    info->deletionSource = NULL;

    bfree(info->data);
    info->data = NULL;

    bfree(info);
    info = NULL;

    obs_log(LOG_INFO, "Source Destroyed");
}

// get the width needed for the source
static uint32_t git_stats_width(void* data) {
    struct gitStatsInfo* info = data;

    return (obs_source_get_width(info->deletionSource));
}

// get the height needed for the source
static uint32_t git_stats_height(void* data) {
    struct gitStatsInfo* info = data;
    if (info->data->insertionEnabled) {
        return (obs_source_get_height(info->insertionSource));
    }
    return (obs_source_get_height(info->deletionSource));
}

static void git_stats_get_defaults(obs_data_t* settings) {
    // repo settings
    obs_data_set_default_int(settings, "delay", 5);
    obs_data_set_default_bool(settings, "untracked_files", false);

    // shared settings
    obs_data_set_default_bool(settings, "outline", false);
    obs_data_set_default_bool(settings, "antialiasing", true);
    obs_data_set_default_bool(settings, "drop_shadow", false);

    // deletion opts
    obs_data_set_default_int(settings, "deletion_color1", 0xFF0000FF);
    obs_data_set_default_int(settings, "deletion_color2", 0xFF0000FF);
    obs_data_set_default_bool(settings, "deletion_symbol", true);

    // insertion opts
    obs_data_set_default_bool(settings, "insertion_symbol", true);
    obs_data_set_default_int(settings, "color1", 0xFF00FF00);
    obs_data_set_default_int(settings, "color2", 0xFF00FF00);

    // make DejaVu Sans Mono the default because sans serif is not mono
    obs_data_t* font_obj = obs_data_create();
    obs_data_set_default_string(font_obj, "face", "DejaVu Sans Mono");
    obs_data_set_default_int(font_obj, "size", 256);
    obs_data_set_default_int(font_obj, "flags", 0);
    obs_data_set_default_string(font_obj, "style", "");
    obs_data_set_default_obj(settings, "font", font_obj);

    // group settings
    obs_data_set_default_bool(settings, "text_properties", true);
    obs_data_set_default_bool(settings, "deletion_properties", true);
}

// this runs when you update settings
static void git_stats_update(void* data, obs_data_t* settings) {
    struct gitStatsInfo* info = data;
    UNUSED_PARAMETER(data);

    // copy settings from dummy props to the deletion source
    obs_data_set_obj(
        info->deletionSource->context.settings, "font",
        obs_data_get_obj(settings, "font"));
    obs_data_set_bool(
        info->deletionSource->context.settings, "antialiasing",
        obs_data_get_bool(settings, "antialiasing"));
    obs_data_set_int(
        info->deletionSource->context.settings, "color1",
        obs_data_get_int(settings, "deletion_color1"));
    obs_data_set_int(
        info->deletionSource->context.settings, "color2",
        obs_data_get_int(settings, "deletion_color2"));
    obs_data_set_bool(
        info->deletionSource->context.settings, "outline",
        obs_data_get_bool(settings, "outline"));
    obs_data_set_bool(
        info->deletionSource->context.settings, "drop_shadow",
        obs_data_get_bool(settings, "drop_shadow"));

    if (!obs_data_get_bool(settings, "insertion_properties")) {
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

    obs_data_array_t* dirArray =
        obs_data_get_array(info->gitSource->context.settings, "single_repos");

    if (!obs_data_array_count(dirArray) &&
        (obs_data_get_string(settings, "repoDir") == NULL ||
         !strcmp(obs_data_get_string(settings, "repoDir"), ""))) {
        obs_data_set_string(
            info->insertionSource->context.settings, "text", "\n+0");
        obs_data_set_string(
            info->deletionSource->context.settings, "text", "\n   -0");
        obs_source_update(
            info->insertionSource, info->insertionSource->context.settings);
        obs_source_update(
            info->deletionSource, info->deletionSource->context.settings);
        info->data->trackedPaths = NULL;
        info->data->numTrackedFiles = 0;
    }
    else {
        if (info->data->trackedPaths) {
            for (int i = 0; i < info->data->numTrackedFiles; i++) {
                free(info->data->trackedPaths[i]);
            }
        }
        else {
            info->data->trackedPaths = malloc(sizeof(char*) * MAXNUMPATHS);
        }
        info->data->numTrackedFiles = 0;
        for (size_t i = 0; i < obs_data_array_count(dirArray); i++) {
            const char* currVal =
                obs_data_get_string(obs_data_array_item(dirArray, i), "value");
            info->data->trackedPaths[i] =
                malloc(sizeof(char) * strlen(currVal) + 1);
            strncpy(info->data->trackedPaths[i], currVal, strlen(currVal) + 1);
            info->data->numTrackedFiles++;
        }
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
    obs_source_video_render(info->insertionSource);
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
    info->invalid_time_passed += seconds;
    if (info->time_passed > info->data->delayAmount || INIT_RUN) {
        INIT_RUN |= 1;
        info->time_passed = 0;
        if (testMode) {
            int numOverload = 4;
            char overloadString[6] = " ";
            for (int i = 1; i < numOverload + 1; i++) {
                overloadString[i] = '.';
                overloadString[i + 1] = '\0';
            }
            char buffer[30] = "";
            snprintf(
                buffer, strlen(ltoa(OVERLOAD_VAL)) + strlen(overloadString) + 3,
                "%s\n+%s", overloadString, ltoa(OVERLOAD_VAL));
            obs_data_set_string(
                info->insertionSource->context.settings, "text", buffer);
            obs_source_update(
                info->insertionSource, info->insertionSource->context.settings);

            ////////////////////

            char spaces[7] = "";
            int deletionSize = strlen(ltoa(OVERLOAD_VAL)) + 2;
            for (int i = 0; i < deletionSize; i++) {
                spaces[i] = ' ';
                spaces[i + 1] = '\0';
            }
            snprintf(
                buffer,
                strlen(overloadString) + (strlen(spaces) * 2) +
                    strlen(ltoa(OVERLOAD_VAL)) + 3,
                "%s%s\n%s-%s", spaces, overloadString, spaces,
                ltoa(OVERLOAD_VAL));
            obs_data_set_string(
                info->deletionSource->context.settings, "text", buffer);
            obs_source_update(
                info->deletionSource, info->deletionSource->context.settings);

            return;
        }
        if (info->data->trackedPaths == NULL) {
            if (info->data->insertionEnabled) {
                if (obs_data_get_bool(
                        info->gitSource->context.settings,
                        "insertion_symbol")) {
                    obs_data_set_string(
                        info->insertionSource->context.settings, "text",
                        "\n+0");
                    obs_source_update(
                        info->insertionSource,
                        info->insertionSource->context.settings);
                }
                else {
                    obs_data_set_string(
                        info->insertionSource->context.settings, "text",
                        "\n 0");
                    obs_source_update(
                        info->insertionSource,
                        info->insertionSource->context.settings);
                }
            }
            else {
                obs_data_set_string(
                    info->insertionSource->context.settings, "text", " ");
                obs_source_update(
                    info->insertionSource,
                    info->insertionSource->context.settings);
            }
            if (info->data->deletionEnabled) {
                if (obs_data_get_bool(
                        info->gitSource->context.settings, "deletion_symbol")) {
                    obs_data_set_string(
                        info->deletionSource->context.settings, "text",
                        "\n   -0");
                    obs_source_update(
                        info->deletionSource,
                        info->deletionSource->context.settings);
                }
                else {
                    obs_data_set_string(
                        info->deletionSource->context.settings, "text",
                        "\n    0");
                    obs_source_update(
                        info->deletionSource,
                        info->deletionSource->context.settings);
                }
            }
            else {
                obs_data_set_string(
                    info->deletionSource->context.settings, "text", "     ");
                obs_source_update(
                    info->deletionSource,
                    info->deletionSource->context.settings);
            }
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
        if (info->data->insertionEnabled) {
            // create overload string
            long value = info->data->added;
            int numOverload = value / OVERLOAD_VAL;
            value = value % OVERLOAD_VAL;
            char overloadString[6] = " ";
            for (int i = 1; i < numOverload + 1; i++) {
                overloadString[i] = '.';
                overloadString[i + 1] = '\0';
            }

            char outputBuffer[100] = "\0";
            if (obs_data_get_bool(
                    info->gitSource->context.settings, "insertion_symbol")) {
                snprintf(
                    outputBuffer,
                    strlen(overloadString) + strlen(ltoa(value)) + 3, "%s\n+%s",
                    overloadString, ltoa(value));
                obs_data_set_string(
                    info->insertionSource->context.settings, "text",
                    outputBuffer);
                obs_source_update(
                    info->insertionSource,
                    info->insertionSource->context.settings);
            }
            else {
                snprintf(
                    outputBuffer,
                    strlen(overloadString) + strlen(ltoa(value)) + 3, "%s\n %s",
                    overloadString, ltoa(value));
                obs_data_set_string(
                    info->insertionSource->context.settings, "text",
                    outputBuffer);
                obs_source_update(
                    info->insertionSource,
                    info->insertionSource->context.settings);
            }
        }
        else {
            char outputBuffer[100] = "\0";
            snprintf(outputBuffer, strlen("") + 1, "%s", "");
            obs_data_set_string(
                info->insertionSource->context.settings, "text", outputBuffer);
            obs_source_update(
                info->insertionSource, info->insertionSource->context.settings);
        }
        char outputBuffer[100] = "\0";
        if (info->data->deletionEnabled) {
            long value = info->data->deleted;
            int numOverload = value / OVERLOAD_VAL;
            value = value % OVERLOAD_VAL;
            char overloadString[6] = " ";
            for (int i = 1; i < numOverload + 1; i++) {
                overloadString[i] = '.';
                overloadString[i + 1] = '\0';
            }
            char spaces[7] = "";
            int insertionSize = strlen(ltoa(info->data->added)) + 2;
            for (int i = 0; i < insertionSize; i++) {
                spaces[i] = ' ';
                spaces[i + 1] = '\0';
            }
            if (obs_data_get_bool(
                    info->gitSource->context.settings, "deletion_symbol")) {
                snprintf(
                    outputBuffer,
                    strlen(overloadString) + (strlen(spaces) * 2) +
                        strlen(ltoa(info->data->deleted)) + 3,
                    "%s%s\n%s-%s", spaces, overloadString, spaces,
                    ltoa(info->data->deleted));
                obs_data_set_string(
                    info->deletionSource->context.settings, "text",
                    outputBuffer);
                obs_source_update(
                    info->deletionSource,
                    info->deletionSource->context.settings);
            }
            else {
                snprintf(
                    outputBuffer,
                    strlen(overloadString) + (strlen(spaces) * 2) +
                        strlen(ltoa(info->data->deleted)) + 3,
                    "%s%s\n%s %s", spaces, overloadString, spaces,
                    ltoa(info->data->deleted));
                obs_data_set_string(
                    info->deletionSource->context.settings, "text",
                    outputBuffer);
                obs_source_update(
                    info->deletionSource,
                    info->deletionSource->context.settings);
            }
        }
        else {
            char spaces[7] = "";
            int insertionSize = strlen(ltoa(info->data->added)) + 2;
            for (int i = 0; i < insertionSize; i++) {
                spaces[i] = ' ';
                spaces[i + 1] = '\0';
            }
            snprintf(
                outputBuffer, strlen(spaces) + strlen(" ") + 1, "%s%s", spaces,
                " ");
            obs_data_set_string(
                info->deletionSource->context.settings, "text", outputBuffer);
            obs_source_update(
                info->deletionSource, info->deletionSource->context.settings);
        }
    }
}

// Toggle test setting
static bool toggleTestCallback(
    obs_properties_t* properties, obs_property_t* buttonProps, void* data) {
    UNUSED_PARAMETER(properties);
    UNUSED_PARAMETER(buttonProps);
    UNUSED_PARAMETER(data);
    testMode ^= 1;
    return (true);
}

// what autogenerates the UI that I can get user data from
static obs_properties_t* git_stats_properties(void* unused) {
    struct gitStatsInfo* info = unused;
    UNUSED_PARAMETER(unused);
    obs_properties_t* props = obs_properties_create();

    obs_properties_t* repo_props = obs_properties_create();

    obs_properties_add_path(
        repo_props, "Repository Directory", "Directory Holding Repositories",
        OBS_PATH_DIRECTORY, NULL, NULL);

    obs_properties_add_editable_list(
        repo_props, "single_repos", "Single Repositories",
        OBS_EDITABLE_LIST_TYPE_FILES, NULL, NULL);

    obs_properties_add_int(
        repo_props, "delay", "Delay Between Updates", 0, INT_MAX, 1);

    obs_properties_add_bool(
        repo_props, "untracked_files", "Account For Untracked Files");

    obs_properties_add_button(
        repo_props, "test_button", "Test Max Size", toggleTestCallback);

    obs_properties_add_group(
        props, "repo_properties", "Repository Settings", OBS_GROUP_NORMAL,
        repo_props);

    ///////////////

    obs_properties_t* shared_props =
        obs_source_properties(info->insertionSource);
    obs_properties_remove_by_name(shared_props, "text_file");
    obs_properties_remove_by_name(shared_props, "from_file");
    obs_properties_remove_by_name(shared_props, "log_mode");
    obs_properties_remove_by_name(shared_props, "log_lines");
    obs_properties_remove_by_name(shared_props, "word_wrap");
    obs_properties_remove_by_name(shared_props, "text");
    obs_properties_remove_by_name(shared_props, "custom_width");
    obs_properties_remove_by_name(shared_props, "color1");
    obs_properties_remove_by_name(shared_props, "color2");
    obs_properties_add_group(
        props, "shared_properties", "Shared Settings", OBS_GROUP_NORMAL,
        shared_props);

    ///////////////

    obs_properties_t* text1_props =
        obs_source_properties(info->insertionSource);
    obs_properties_remove_by_name(text1_props, "font");
    obs_properties_remove_by_name(text1_props, "text_file");
    obs_properties_remove_by_name(text1_props, "from_file");
    obs_properties_remove_by_name(text1_props, "log_mode");
    obs_properties_remove_by_name(text1_props, "log_lines");
    obs_properties_remove_by_name(text1_props, "word_wrap");
    obs_properties_remove_by_name(text1_props, "text");
    obs_properties_remove_by_name(text1_props, "custom_width");
    obs_properties_remove_by_name(text1_props, "drop_shadow");
    obs_properties_remove_by_name(text1_props, "outline");
    obs_properties_remove_by_name(text1_props, "antialiasing");
    obs_properties_add_bool(text1_props, "insertion_symbol", "+ Symbol");
    obs_data_set_default_int(
        info->insertionSource->context.settings, "color1", 0xFF00FF00);
    obs_data_set_default_int(
        info->insertionSource->context.settings, "color1", 0xFF00FF00);
    obs_properties_add_group(
        props, "insertion_properties", "Insertion Settings",
        OBS_GROUP_CHECKABLE, text1_props);

    //////////////

    obs_properties_t* text2_props = obs_properties_create();
    obs_properties_add_color_alpha(text2_props, "deletion_color1", "Color1");
    obs_properties_add_color_alpha(text2_props, "deletion_color2", "Color2");
    obs_properties_add_bool(text2_props, "deletion_symbol", "- Symbol");

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
        .get_name     = git_stats_name,
        .create       = git_stats_create,
        .destroy      = git_stats_destroy,
        .update       = git_stats_update,
        .video_render = git_stats_render,
        .get_width    = git_stats_width,
        .get_height   = git_stats_height,
        .video_tick = git_stats_tick, 
        .get_properties = git_stats_properties, 
        .icon_type = OBS_ICON_TYPE_TEXT, 
    };
// clang-format on
