#include "./git-stats-source.h"
#include "git2.h" 

struct git_stats_info{
  // internal text sources
	obs_source_t *ft2_insertion;
	obs_source_t *ft2_deletion;
  
  //this source
	obs_source_t *git_stats;

	uint32_t len_x;
	uint32_t len_y;

	// the information that we get from our git stats thingy madonker
	struct git_stats_data *data;
};

static const char *git_stats_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return (obs_module_text("Git Stats"));
}

// update the settings data in our source
static void git_stats_update(void *data, obs_data_t *settings)
{
  //TODO: create git diff options here, signal to thread to update
  // immediately
  UNUSED_PARAMETER(settings);
  UNUSED_PARAMETER(data);
}

// initializes the source
static void *git_stats_create(obs_data_t *settings, obs_source_t *source)
{ 
  struct git_stats_info *ctx = bzalloc(sizeof(struct git_stats_info));
  ctx->git_stats = source; 

  //TODO: see if having these settings set to git-stats settings allows them to
  //be controlled through git-stats. If not keep their settings in 
  //git_stats_data and call update for them in git_stats_update on change
  //(try setting the properties to the exact same name)
  ctx->ft2_insertion = obs_source_create_private("text_ft2_source_v2", 
                                                 "insertion_source", 
                                                 NULL);
	ctx->ft2_deletion = obs_source_create_private("text_ft2_source_v2", 
                                                "deletion_source", 
                                                NULL);

  git_stats_update(ctx, settings);
  return(ctx);
}

static void git_stats_destroy(void *data)
{
  struct git_stats_info* ctx = data;
  (void)obs_source_release(ctx->ft2_insertion);
  (void)obs_source_release(ctx->ft2_deletion);
  //TODO: close thread, destroy mutex and sem
  bfree(ctx);
}

// get the width needed for the source
static uint32_t git_stats_width(void *data)
{
  // deletion will be the length of the entire source so we take that width
  struct git_stats_info* ctx = data;
  return(obs_source_get_width(ctx->ft2_deletion));
}

// get the height needed for the source
static uint32_t git_stats_height(void *data)
{
  struct git_stats_info* ctx = data;
  return(obs_source_get_width(ctx->ft2_deletion));
}

// Settings that are loaded when the default button is hit in the properties
// window
static void git_stats_get_defaults(obs_data_t *settings){
  UNUSED_PARAMETER(settings);
}

static obs_properties_t *git_stats_properties(void *data)
{
  struct git_stats_info* ctx = data;

  obs_properties_t *properties= obs_properties_create();
  obs_properties_t *gen_properties = obs_properties_create();

  (void)obs_properties_add_path(gen_properties, "repo", 
                          obs_module_text("Repository"), 
                          OBS_PATH_DIRECTORY, NULL, NULL);  
 
  obs_property_t* list = obs_properties_add_list(gen_properties, 
                          "speed",
                          obs_module_text("Speed At Which To Update The Diff"),
                          OBS_COMBO_TYPE_RADIO, OBS_COMBO_FORMAT_INT);
  
  (void)obs_property_list_add_int(list, obs_module_text("Slow"), 10); 
  (void)obs_property_list_add_int(list, obs_module_text("Normal"), 5); 
  (void)obs_property_list_add_int(list, obs_module_text("Fast"), 3); 
  (void)obs_properties_add_group(properties, "gen_properties", 
                           "General Settings", OBS_GROUP_NORMAL, 
                           gen_properties);
  
  //---------------------------------------------------------------------------
  
  obs_properties_t *shared_text_properties = 
    obs_source_properties(ctx->ft2_insertion);

  (void)obs_properties_remove_by_name(shared_text_properties, "text_file");
	(void)obs_properties_remove_by_name(shared_text_properties, "from_file");
	(void)obs_properties_remove_by_name(shared_text_properties, "log_mode");
	(void)obs_properties_remove_by_name(shared_text_properties, "log_lines");
	(void)obs_properties_remove_by_name(shared_text_properties, "word_wrap");
	(void)obs_properties_remove_by_name(shared_text_properties, "text");
	(void)obs_properties_remove_by_name(shared_text_properties, "custom_width");
	(void)obs_properties_remove_by_name(shared_text_properties, "color1");
	(void)obs_properties_remove_by_name(shared_text_properties, "color2");
  (void)obs_properties_add_group(properties, "shared_text_properties", 
                                 "Shared Settings", OBS_GROUP_NORMAL, 
                                 shared_text_properties);
  
  //---------------------------------------------------------------------------
  
  obs_properties_t* insertion_properties = 
    obs_source_properties(ctx->ft2_insertion);

  (void)obs_properties_remove_by_name(insertion_properties, "font");
	(void)obs_properties_remove_by_name(insertion_properties, "text_file");
	(void)obs_properties_remove_by_name(insertion_properties, "from_file");
	(void)obs_properties_remove_by_name(insertion_properties, "log_mode");
	(void)obs_properties_remove_by_name(insertion_properties, "log_lines");
	(void)obs_properties_remove_by_name(insertion_properties, "word_wrap");
	(void)obs_properties_remove_by_name(insertion_properties, "text");
	(void)obs_properties_remove_by_name(insertion_properties, "custom_width");
	(void)obs_properties_remove_by_name(insertion_properties, "drop_shadow");
	(void)obs_properties_remove_by_name(insertion_properties, "outline");
	(void)obs_properties_remove_by_name(insertion_properties, "antialiasing");
	(void)obs_properties_add_bool(insertion_properties, 
                         "insertion_symbol", "+ Symbol");

  (void)obs_properties_add_group(properties, "insertion_properties", 
                           "Insertion Settings", OBS_GROUP_CHECKABLE,
                           insertion_properties);


  //---------------------------------------------------------------------------
  
  obs_properties_t* deletion_properties = obs_properties_create();
  (void)obs_properties_add_color_alpha(deletion_properties, "deletion_color1", 
                                 "Color1");
  (void)obs_properties_add_color_alpha(deletion_properties, "deletion_color2", 
                                 "Color2");
  
  (void)obs_properties_add_group(properties, "deletion_properties", 
                           "Deletion Settings", OBS_GROUP_CHECKABLE,
                           deletion_properties);
	return properties;
}

// render out the source
static void git_stats_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
  struct git_stats_info* ctx = data;
  obs_source_video_render(ctx->ft2_insertion);
  obs_source_video_render(ctx->ft2_deletion);
}

static void git_stats_tick(void *data, float seconds){
  //TODO: Check sem, if val then create and update output strings
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
