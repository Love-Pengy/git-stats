#include "./git-stats-source.h"
#include "git2.h" 

static const char *git_stats_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return (obs_module_text("Git Stats"));
}

// initializes the source
static void *git_stats_create(obs_data_t *settings, obs_source_t *source)
{

  if (0 > git_libgit2_init()) {
    blog(LOG_ERROR, "git_libgit2_init");
  }
  blog(LOG_INFO, "libgit2 init success");

  if (0 > git_libgit2_shutdown()) {
      blog(LOG_ERROR, "git_libgit2_shutdown"); 
  }
  blog(LOG_INFO, "libgit2 shutdown success");
  
  UNUSED_PARAMETER(settings);
  UNUSED_PARAMETER(source);
	blog(LOG_INFO, "Source Created");
  return(NULL);
}

// bfree up the source and its data
static void git_stats_destroy(void *data)
{
  UNUSED_PARAMETER(data);
	blog(LOG_INFO, "Source Destroyed");
}

// get the width needed for the source
static uint32_t git_stats_width(void *data)
{
  UNUSED_PARAMETER(data);
	return (0);
}

// get the height needed for the source
static uint32_t git_stats_height(void *data)
{
  UNUSED_PARAMETER(data);
	return (0);
}

// Settings that are loaded when the default button is hit in the properties
// window
static void git_stats_get_defaults(obs_data_t *settings){
  UNUSED_PARAMETER(settings);
}

// update the settings data in our source
static void git_stats_update(void *data, obs_data_t *settings)
{
  UNUSED_PARAMETER(settings);
  UNUSED_PARAMETER(data);
}

// properties that are generated in the ui of the source
static obs_properties_t *git_stats_properties(void *unused)
{
  UNUSED_PARAMETER(unused);
	return NULL;
}
// render out the source
static void git_stats_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
  UNUSED_PARAMETER(data);
}

static void git_stats_tick(void *data, float seconds){
  UNUSED_PARAMETER(data);
  UNUSED_PARAMETER(seconds);
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
