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

static void git_stats_update(void *data, obs_data_t *settings)
{
  //TODO: create git diff options here, signal to thread to update
  // immediately
  //
  // TODO: also create properties for insertion and deletion and update them with 
  // those settings
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

// these should stop or resume git diff updates
static void git_stats_show(void* data){
  UNUSED_PARAMETER(data);
};
static void git_stats_hide(void* data){
  UNUSED_PARAMETER(data);
};

static void git_stats_defaults(obs_data_t *settings){

  obs_data_set_default_int(settings, "speed", 5);

  obs_data_t* default_font = obs_data_create();
	obs_data_set_default_string(default_font, "face", "DejaVu Sans Mono");
	obs_data_set_default_int(default_font, "size", 256);
	obs_data_set_default_int(default_font, "flags", 0);
	obs_data_set_default_string(default_font, "style", "");
  obs_data_set_default_obj(settings, "font" , default_font);
  
  obs_data_set_default_string(settings, "overload_char", "."); 
  
  obs_data_set_default_bool(settings, "antialiasing", true);
  obs_data_set_default_bool(settings, "outline", false);
  obs_data_set_default_bool(settings, "drop_shadow", false);
  
  obs_data_set_default_bool(settings, "deletion_properties", true); 
  obs_data_set_default_int(settings, "deletion_color1", 0xFF0000FF);
	obs_data_set_default_int(settings, "deletion_color2", 0xFF0000FF);
	obs_data_set_default_bool(settings, "deletion_symbol", true);

  obs_data_set_default_bool(settings, "insertion_properties", true); 
  obs_data_set_default_int(settings, "insertion_color1", 0xFF00FF00);
	obs_data_set_default_int(settings, "insertion_color2", 0xFF00FF00);
	obs_data_set_default_bool(settings, "insertion_symbol", true);

}


static obs_properties_t *git_stats_properties(void *data)
{
  UNUSED_PARAMETER(data);

  obs_properties_t *properties= obs_properties_create();
  // wait until user is done changing settings to update plugin
  obs_properties_set_flags(properties,OBS_PROPERTIES_DEFER_UPDATE);
  
  obs_properties_t* general_properties = obs_properties_create();

  (void)obs_properties_add_path(general_properties, "repo", 
                          obs_module_text("Repository"), 
                          OBS_PATH_DIRECTORY, NULL, NULL);  
  (void)obs_properties_add_int(general_properties, "speed", 
                               obs_module_text("Update Speed"),
                               3, INT_MAX, 1);
  (void)obs_properties_add_text(general_properties, "overload_char", 
                                obs_module_text("Overload Character"), 
                                OBS_TEXT_DEFAULT);
  (void)obs_properties_add_text(general_properties, "overload_info", 
                                "ASCII or Unicode Characters May Be Specified",
                                OBS_TEXT_INFO);
  (void)obs_properties_add_font(general_properties, "font", 
                                obs_module_text("Font")); 

  (void)obs_properties_add_bool(general_properties, "antialiasing", 
                                obs_module_text("Antialiasing"));
  (void)obs_properties_add_bool(general_properties, "outline", 
                                obs_module_text("Outline"));
  (void)obs_properties_add_bool(general_properties, "drop_shadow", 
                                obs_module_text("Drow Shadows"));

  (void)obs_properties_add_group(properties, "general_properties", 
                           obs_module_text("General Settings"), 
                           OBS_GROUP_NORMAL, general_properties);

  //---------------------------------------------------------------------------
  
  obs_properties_t* insertion_properties = obs_properties_create();
  (void)obs_properties_add_color_alpha(insertion_properties, 
                                       "insertion_color1",
                                       obs_module_text("Color 1"));
  (void)obs_properties_add_color_alpha(insertion_properties, 
                                       "insertion_color2",
                                       obs_module_text("Color 2"));
	(void)obs_properties_add_bool(insertion_properties, 
                                "insertion_symbol", 
                                obs_module_text("+ Symbol"));

  (void)obs_properties_add_group(properties, "insertion_properties", 
                                 obs_module_text("Insertion Settings"), 
                                 OBS_GROUP_CHECKABLE, insertion_properties);

  //---------------------------------------------------------------------------
  
  obs_properties_t* deletion_properties = obs_properties_create();
  (void)obs_properties_add_color_alpha(deletion_properties, "deletion_color1", 
                                 obs_module_text("Color 1"));
  (void)obs_properties_add_color_alpha(deletion_properties, "deletion_color2", 
                                 obs_module_text("Color 2"));
	(void)obs_properties_add_bool(deletion_properties, "deletion_symbol", 
                                obs_module_text("- Symbol"));
  
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
        .id             = "git-stats",
        .type           = OBS_SOURCE_TYPE_INPUT,
        .output_flags   = OBS_SOURCE_VIDEO,
        .get_name       = git_stats_name,
        .create         = git_stats_create,
        .destroy        = git_stats_destroy,
        .update         = git_stats_update,
        .video_render   = git_stats_render,
        .get_width      = git_stats_width,
        .get_height     = git_stats_height,
        .get_defaults   = git_stats_defaults,
        .video_tick     = git_stats_tick, 
        .get_properties = git_stats_properties, 
        .hide           = git_stats_hide, 
        .show           = git_stats_show,
        .icon_type = OBS_ICON_TYPE_TEXT, 
    };
// clang-format on
