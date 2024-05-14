#include <obs-internal.h>
#include <obs-module.h>
#include <obs-source.h>

#include "support.h"

int TEST_THRESHOLD = 10000;

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

    // the information that we get from our git stats thingy madonker
    struct gitData* data;
};

static const char* git_stats_name(void* unused) {
    UNUSED_PARAMETER(unused);
    return (obs_module_text("GitStats"));
}

static void* git_stats_create(obs_data_t* settings, obs_source_t* source) {
    struct gitStatsInfo* info = bzalloc(sizeof(struct gitStatsInfo));
    info->time_passed = 0;
    info->gitSource = source;

    // id for the text source
    const char* text_source_id = "text_ft2_source\0";
    info->textSource =
        obs_source_create(text_source_id, text_source_id, settings, NULL);
    obs_source_add_active_child(info->gitSource, info->textSource);

    return (info);
}

static void git_stats_destroy(void* data) {
    struct gitStatsInfo* info = data;

    obs_source_remove(info->textSource);
    obs_source_release(info->textSource);
    info->textSource = NULL;

    bfree(info->data);
    info->data = NULL;

    bfree(info);
}

static uint32_t git_stats_width(void* data) {
    struct gitStatsInfo* info = data;

    return (obs_source_get_width(info->textSource));
}

static uint32_t git_stats_height(void* data) {
    struct gitStatsInfo* info = data;

    return (obs_source_get_height(info->textSource));
}

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

static void git_stats_render(void* data, gs_effect_t* effect) {
    obs_log(LOG_INFO, "FRAME RENDERED");
    struct gitStatsInfo* info = data;
    obs_source_video_render(info->textSource);
    UNUSED_PARAMETER(effect);
}

// called each frame with the time elapsed passed in
static void git_stats_tick(void* data, float seconds) {
    struct gitStatsInfo* info = data;

    // don't update if the source isn't active
    if (!obs_source_showing(info->gitSource)) {
        return;
    }

    info->time_passed += seconds;
    if (((int)info->time_passed % TEST_THRESHOLD) == 0) {
        obs_data_set_string(info->textSource->context.settings, "text", "one");
    }
    else {
        obs_log(LOG_INFO, "PLUGIN TICKED");
        obs_data_set_string(info->textSource->context.settings, "text", "two");
        info->time_passed = 0;
    }
}

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
