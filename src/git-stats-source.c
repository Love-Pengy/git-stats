#include <obs-internal.h>
#include <obs-module.h>
#include <obs-source.h>

#include "support.h"

struct gitData {
    long added;
    long deleted;
    char** paths;
};

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

    int test;

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
    info->test = 0;
    // the source itself
    info->gitSource = source;

    // id for the text source
    const char* text_source_id = "text_ft2_source\0";
    // the text source
    info->textSource =
        obs_source_create(text_source_id, text_source_id, settings, NULL);
    obs_source_add_active_child(info->gitSource, info->textSource);

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
    obs_data_set_default_bool(settings, "unload", true);
    obs_data_set_default_bool(settings, "topheartrate", false);
    obs_data_set_default_string(settings, "comport", "COM4");
}

// this runs when you update settings
static void git_stats_update(void* data, obs_data_t* settings) {
    struct gitStatsInfo* info = data;

    obs_data_set_string(info->textSource->context.settings, "text", "N/A");
    UNUSED_PARAMETER(settings);
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
    info->test += 1;

    if (info->test > 500) {
        obs_log(LOG_INFO, "%d", info->test);
        // set the string in the text source
        obs_data_set_string(info->textSource->context.settings, "text", "two");
    }
    else {
        obs_data_set_string(info->textSource->context.settings, "text", "one");
    }
    // force update the source (not calling this will keep the state of the
    // previous value
    obs_source_update(info->textSource, info->textSource->context.settings);
}

// what autogenerates the UI that I can get user data from (learn about this)
static obs_properties_t* git_stats_properties(void* unused) {
    struct gitStatsInfo* info = unused;

    obs_properties_t* props = obs_source_properties(info->textSource);

    obs_properties_add_text(props, "comport", "COM port", OBS_TEXT_DEFAULT);

    obs_properties_add_bool(
        props, "topheartrate", "Show top heart rate record");

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
