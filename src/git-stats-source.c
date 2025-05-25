#include <unistd.h>
#include <obs-module.h>
#include <util/threading.h>
#include "git2.h"
#include "support.h"
#include "cmake_vars.h"

#define MAX_DIGITS 4
// + 3 for insertion symbol, space, and null byte
#define SPACES_SIZE MAX_DIGITS + 3
//compile time generation of spaces string
//https://gcc.gnu.org/onlinedocs/gcc/Designated-Inits.html
const char SPACES[SPACES_SIZE] = {[0 ... SPACES_SIZE - 2] = ' ', [SPACES_SIZE - 1] = '\0'};

#define MAX_VAL generate_max_int(MAX_DIGITS)

// Generates Constant At Compile Time
// https://gcc.gnu.org/onlinedocs/gcc/Inline.html
static inline long __attribute__((always_inline)) generate_max_int(const int n)
{
	long result = 0;
	for (int i = 0; i < n; i++) {
		result = (result * 10) + 9;
	}
	return result;
}

struct git_thread_ctx {
	// running context
	bool initialized;
	git_repository *repo;

	// settings
	int update_speed;
	const char *repository_path;
	bool new_val_status;
	bool update_request;
	bool active;
	bool exit;
	pthread_cond_t sleep_interrupt;
	pthread_mutex_t data_mutex;
	git_diff_options opts;

	// values
	uint insertions;
	uint deletions;
};

struct git_stats_info {
	// internal text sources
	obs_source_t *ft2_insertion;
	obs_source_t *ft2_deletion;

	obs_source_t *git_stats;

	//text settings for constructing output
	char *overload_char;
	uint32_t len_x;
	uint32_t len_y;
	bool insertion_symbol_en;
	bool deletion_symbol_en;

	pthread_t git_thread;

	struct git_thread_ctx *thread_ctx;
};

///////////////
// THREADING //
///////////////

// Waits for either a signal on
void wait_sig_time(struct git_thread_ctx *thread_data)
{
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);
	spec.tv_sec += thread_data->update_speed;

	(void)pthread_mutex_lock(&(thread_data->data_mutex));

	(void)pthread_cond_timedwait(&(thread_data->sleep_interrupt), &(thread_data->data_mutex), &spec);

	(void)pthread_mutex_unlock(&(thread_data->data_mutex));
}

void *libgit2_thread(void *data)
{
	struct git_thread_ctx *thread_data = data;
	os_set_thread_name("libgit2");
	while (!thread_data->exit) {
		if (!(thread_data->active)) {
			usleep(100);
			continue;
		}
		if (thread_data->update_request) {
			if (!(thread_data->initialized)) {
				(void)git_libgit2_init();
				thread_data->initialized = true;
			}

			if (thread_data->repo) {
				git_repository_free(thread_data->repo);
			}

			(void)pthread_mutex_lock(&(thread_data->data_mutex));

			(void)git_repository_open(&(thread_data->repo), thread_data->repository_path);
		}

		git_diff *diff = NULL;
		git_diff_stats *stats = NULL;
		(void)git_diff_index_to_workdir(&diff, thread_data->repo, NULL, &(thread_data->opts));
		(void)git_diff_get_stats(&stats, diff);
		thread_data->insertions = git_diff_stats_insertions(stats);
		thread_data->deletions = git_diff_stats_deletions(stats);
		thread_data->update_request = false;
		thread_data->new_val_status = true;

#ifdef TEST_MODE
		info("[git-stats] [TEST] %d %d", thread_data->insertions, thread_data->deletions);
#endif

		(void)pthread_mutex_unlock(&(thread_data->data_mutex));
		git_diff_stats_free(stats);
		git_diff_free(diff);

		wait_sig_time(thread_data);
	}
	git_repository_free(thread_data->repo);
	git_libgit2_shutdown();
	pthread_exit(0);
}

static const char *git_stats_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return (obs_module_text("Git Stats"));
}

static void git_stats_update(void *data, obs_data_t *settings)
{
	struct git_stats_info *ctx = data;

	pthread_mutex_lock(&(ctx->thread_ctx->data_mutex));
	ctx->thread_ctx->update_speed = obs_data_get_int(settings, "speed");
	ctx->thread_ctx->repository_path = obs_data_get_string(settings, "repo");
	pthread_mutex_unlock(&(ctx->thread_ctx->data_mutex));

	ctx->insertion_symbol_en = obs_data_get_bool(settings, "insertion_symbol");
	ctx->deletion_symbol_en = obs_data_get_bool(settings, "deletion_symbol");

	bfree(ctx->overload_char);
	ctx->overload_char = extract_unicode(obs_data_get_string(settings, "overload_char"));

	obs_data_t *font_obj = obs_data_get_obj(settings, "font");

	// Insertion Portion
	obs_data_t *insertion_settings = obs_source_get_settings(ctx->ft2_insertion);

	obs_data_set_obj(insertion_settings, "font", font_obj);

	obs_data_set_bool(insertion_settings, "antialiasing", obs_data_get_bool(settings, "antialiasing"));
	obs_data_set_bool(insertion_settings, "outline", obs_data_get_bool(settings, "outline"));
	obs_data_set_bool(insertion_settings, "drop_shadow", obs_data_get_bool(settings, "drop_shadow"));
	obs_data_set_int(insertion_settings, "color1", obs_data_get_int(settings, "insertion_color1"));
	obs_data_set_int(insertion_settings, "color2", obs_data_get_int(settings, "insertion_color2"));
	obs_source_update(ctx->ft2_insertion, insertion_settings);
	obs_data_release(insertion_settings);

	// Deletion Portion
	obs_data_t *deletion_settings = obs_source_get_settings(ctx->ft2_deletion);

	obs_data_set_obj(deletion_settings, "font", font_obj);
	obs_data_release(font_obj);

	obs_data_set_bool(deletion_settings, "antialiasing", obs_data_get_bool(settings, "antialiasing"));
	obs_data_set_bool(deletion_settings, "outline", obs_data_get_bool(settings, "outline"));
	obs_data_set_bool(deletion_settings, "drop_shadow", obs_data_get_bool(settings, "drop_shadow"));
	obs_data_set_int(deletion_settings, "color1", obs_data_get_int(settings, "deletion_color1"));
	obs_data_set_int(deletion_settings, "color2", obs_data_get_int(settings, "deletion_color2"));
	obs_source_update(ctx->ft2_deletion, deletion_settings);
	obs_data_release(deletion_settings);

	(void)pthread_mutex_lock(&(ctx->thread_ctx->data_mutex));

	git_diff_options git_opts;
	(void)git_diff_options_init(&git_opts, GIT_DIFF_OPTIONS_VERSION);

	obs_data_get_bool(settings, "untracked")
		? git_opts.flags = (GIT_DIFF_SHOW_UNTRACKED_CONTENT | GIT_DIFF_RECURSE_UNTRACKED_DIRS)
		: 0;

	ctx->thread_ctx->opts = git_opts;
	ctx->thread_ctx->update_request = true;
	(void)pthread_cond_broadcast(&(ctx->thread_ctx->sleep_interrupt));

	(void)pthread_mutex_unlock(&(ctx->thread_ctx->data_mutex));
}

// initializes the source
static void *git_stats_create(obs_data_t *settings, obs_source_t *source)
{
	struct git_stats_info *ctx = bzalloc(sizeof(struct git_stats_info));
	ctx->thread_ctx = bzalloc(sizeof(struct git_thread_ctx));
	ctx->git_stats = source;
	ctx->ft2_insertion = obs_source_create_private("text_ft2_source_v2", "insertion_source", NULL);
	ctx->ft2_deletion = obs_source_create_private("text_ft2_source_v2", "deletion_source", NULL);

	git_stats_update(ctx, settings);

	(void)pthread_cond_init(&(ctx->thread_ctx->sleep_interrupt), NULL);

	(void)pthread_mutex_init(&(ctx->thread_ctx->data_mutex), NULL);
	(void)pthread_create(&(ctx->git_thread), NULL, libgit2_thread, ctx->thread_ctx);

	return (ctx);
}

static void git_stats_destroy(void *data)
{
	struct git_stats_info *ctx = data;
	pthread_mutex_lock(&(ctx->thread_ctx->data_mutex));
	ctx->thread_ctx->exit = true;
	pthread_mutex_unlock(&(ctx->thread_ctx->data_mutex));
	(void)pthread_cond_broadcast(&(ctx->thread_ctx->sleep_interrupt));
	(void)obs_source_release(ctx->ft2_insertion);
	(void)obs_source_release(ctx->ft2_deletion);
	(void)pthread_join(ctx->git_thread, NULL);
	(void)pthread_mutex_destroy(&(ctx->thread_ctx->data_mutex));
	(void)pthread_cond_destroy(&(ctx->thread_ctx->sleep_interrupt));
	bfree(ctx->overload_char);
	bfree(ctx->thread_ctx);
	bfree(ctx);
}

// get the width needed for the source
static uint32_t git_stats_width(void *data)
{
	// deletion will be the length of the entire source so we take that width
	struct git_stats_info *ctx = data;
	return (obs_source_get_width(ctx->ft2_deletion));
}

// get the height needed for the source
static uint32_t git_stats_height(void *data)
{
	struct git_stats_info *ctx = data;
	return (obs_source_get_height(ctx->ft2_deletion));
}

// these should stop or resume git diff updates
static void git_stats_show(void *data)
{
	struct git_stats_info *ctx = data;
	pthread_mutex_lock(&(ctx->thread_ctx->data_mutex));
	ctx->thread_ctx->active = true;
	ctx->thread_ctx->update_request = true;
	pthread_mutex_unlock(&(ctx->thread_ctx->data_mutex));
};

static void git_stats_hide(void *data)
{
	struct git_stats_info *ctx = data;
	pthread_mutex_lock(&(ctx->thread_ctx->data_mutex));
	ctx->thread_ctx->active = false;
	pthread_mutex_unlock(&(ctx->thread_ctx->data_mutex));
};

static void git_stats_defaults(obs_data_t *settings)
{

	obs_data_set_default_int(settings, "speed", 5);

	obs_data_t *default_font = obs_data_create();
	obs_data_set_default_string(default_font, "face", "DejaVu Sans Mono");
	obs_data_set_default_int(default_font, "size", 256);
	obs_data_set_default_int(default_font, "flags", 0);
	obs_data_set_default_string(default_font, "style", "");
	obs_data_set_default_obj(settings, "font", default_font);
	obs_data_release(default_font);

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

	obs_properties_t *properties = obs_properties_create();

	obs_properties_t *general_properties = obs_properties_create();

	(void)obs_properties_add_path(general_properties, "repo", obs_module_text("Repository"), OBS_PATH_DIRECTORY,
				      NULL, NULL);
	(void)obs_properties_add_int(general_properties, "speed", obs_module_text("Update Speed"), 3, INT_MAX, 1);
	(void)obs_properties_add_text(general_properties, "overload_char", obs_module_text("Overload Character"),
				      OBS_TEXT_DEFAULT);
	(void)obs_properties_add_text(general_properties, "overload_info",
				      "ASCII or Unicode Characters May Be Specified", OBS_TEXT_INFO);
	(void)obs_properties_add_bool(general_properties, "untracked", obs_module_text("Include Untracked Files"));
	//TODO: add advanced section to expose more settings for diff
	(void)obs_properties_add_font(general_properties, "font", obs_module_text("Font"));

	(void)obs_properties_add_bool(general_properties, "antialiasing", obs_module_text("Antialiasing"));
	(void)obs_properties_add_bool(general_properties, "outline", obs_module_text("Outline"));
	(void)obs_properties_add_bool(general_properties, "drop_shadow", obs_module_text("Drow Shadows"));

	(void)obs_properties_add_group(properties, "general_properties", obs_module_text("General Settings"),
				       OBS_GROUP_NORMAL, general_properties);

	//---------------------------------------------------------------------------

	obs_properties_t *insertion_properties = obs_properties_create();
	(void)obs_properties_add_color_alpha(insertion_properties, "insertion_color1", obs_module_text("Color 1"));
	(void)obs_properties_add_color_alpha(insertion_properties, "insertion_color2", obs_module_text("Color 2"));
	(void)obs_properties_add_bool(insertion_properties, "insertion_symbol", obs_module_text("+ Symbol"));

	(void)obs_properties_add_group(properties, "insertion_properties", obs_module_text("Insertion Settings"),
				       OBS_GROUP_CHECKABLE, insertion_properties);

	//---------------------------------------------------------------------------

	obs_properties_t *deletion_properties = obs_properties_create();
	(void)obs_properties_add_color_alpha(deletion_properties, "deletion_color1", obs_module_text("Color 1"));
	(void)obs_properties_add_color_alpha(deletion_properties, "deletion_color2", obs_module_text("Color 2"));
	(void)obs_properties_add_bool(deletion_properties, "deletion_symbol", obs_module_text("- Symbol"));

	(void)obs_properties_add_group(properties, "deletion_properties", "Deletion Settings", OBS_GROUP_CHECKABLE,
				       deletion_properties);
	return properties;
}

// render out the source
static void git_stats_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	struct git_stats_info *ctx = data;
	obs_source_video_render(ctx->ft2_insertion);
	obs_source_video_render(ctx->ft2_deletion);
}

static void git_stats_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(seconds);
#ifdef TEST_MODE
	profile_start("git-stats_tick");
#endif
	struct git_stats_info *ctx = data;
	if (ctx->thread_ctx->new_val_status) {
		obs_data_t *insertion_settings = obs_source_get_settings(ctx->ft2_insertion);
		obs_data_t *deletion_settings = obs_source_get_settings(ctx->ft2_deletion);
		char text_buffer[(((MAX_DIGITS * 4) + 1) + 1 + ((MAX_DIGITS * 4) + 1)) * 2];

		pthread_mutex_lock(&(ctx->thread_ctx->data_mutex));
		uint truncated_insertion_val = ctx->thread_ctx->insertions % MAX_VAL;
		int num_overload_insertion = ctx->thread_ctx->insertions / MAX_VAL;
		pthread_mutex_unlock(&(ctx->thread_ctx->data_mutex));
		num_overload_insertion > MAX_DIGITS ? num_overload_insertion = MAX_DIGITS : num_overload_insertion;

		// max 4 bytes for each unicode char
		char insertion_overload_string[(MAX_DIGITS * 4) + 1];
		insertion_overload_string[0] = ' ';
		insertion_overload_string[1] = '\0';

		// generate insertion overload string
		for (int i = 1; i < num_overload_insertion + 1; i++) {
			strcat(insertion_overload_string, ctx->overload_char);
		}

		// construct insertion string
		char insertion_value_string[MAX_DIGITS + 2];

		if (ctx->insertion_symbol_en) {
			insertion_value_string[0] = '+';
			insertion_value_string[1] = '\0';
		} else {
			insertion_value_string[0] = ' ';
			insertion_value_string[1] = '\0';
		}

		(void)snprintf(insertion_value_string + 1, MAX_DIGITS + 1, "%d", truncated_insertion_val);
		pad_string_left(insertion_value_string, MAX_DIGITS + 1);
		sprintf(text_buffer, "%s\n%s", insertion_overload_string, insertion_value_string);
		obs_data_set_string(insertion_settings, "text", text_buffer);
		obs_source_update(ctx->ft2_insertion, insertion_settings);
		obs_data_release(insertion_settings);

		pthread_mutex_lock(&(ctx->thread_ctx->data_mutex));
		uint truncated_deletion_val = ctx->thread_ctx->deletions % MAX_VAL;
		int num_overload_deletion = ctx->thread_ctx->deletions / MAX_VAL;
		pthread_mutex_unlock(&(ctx->thread_ctx->data_mutex));
		num_overload_deletion > MAX_DIGITS ? num_overload_deletion = MAX_DIGITS : num_overload_deletion;

		char deletion_overload_string[SPACES_SIZE + 1 + ((MAX_DIGITS * 4) + 2)];
		(void)memset(deletion_overload_string, '\0', sizeof(deletion_overload_string));

		strncpy(deletion_overload_string, SPACES, SPACES_SIZE);

		// generate overload string
		for (int i = SPACES_SIZE; i < SPACES_SIZE + num_overload_deletion; i++) {
			strcat(deletion_overload_string, ctx->overload_char);
		}

		char deletion_value_string[SPACES_SIZE + 1 + (MAX_DIGITS + 2)];

		(void)strncpy(deletion_value_string, SPACES, SPACES_SIZE);

		if (ctx->deletion_symbol_en) {
			deletion_value_string[strlen(deletion_value_string)] = '-';
			deletion_value_string[strlen(deletion_value_string) + 1] = '\0';
		} else {
			deletion_value_string[strlen(deletion_value_string)] = ' ';
			deletion_value_string[strlen(deletion_value_string) + 1] = '\0';
		}

		(void)snprintf(deletion_value_string + SPACES_SIZE, MAX_DIGITS + 1, "%d", truncated_deletion_val);
		pad_string_right(deletion_value_string + SPACES_SIZE, MAX_DIGITS + 1);

		sprintf(text_buffer, "%s\n%s", deletion_overload_string, deletion_value_string);

		obs_data_set_string(deletion_settings, "text", text_buffer);
		obs_source_update(ctx->ft2_deletion, deletion_settings);
		obs_data_release(deletion_settings);

		pthread_mutex_lock(&(ctx->thread_ctx->data_mutex));
		ctx->thread_ctx->new_val_status = false;
		pthread_mutex_unlock(&(ctx->thread_ctx->data_mutex));
	}
#ifdef TEST_MODE
	profile_end("git-stats_tick");
	profile_reenable_thread();
#endif
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
        .icon_type      = OBS_ICON_TYPE_TEXT, 
    };
// clang-format on
